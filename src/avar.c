#undef NDEBUG /* always check assertions */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "thread.h"

enum { kStateNone, kStateBusy, kStateRunning, kStateReady, };

typedef struct {
    void *p;
    mutex_t mutex;
    volatile long state;
} once_t;

static void* init_once(once_t *s, size_t arg, void*(*fn)(size_t arg))
{
    long old_state;
    unsigned trys = 2;
    do {
        old_state = s->state;
        switch (old_state) {
        case kStateRunning:
            mutex_lock(&s->mutex);           /* other thread holds lock */
            mutex_unlock(&s->mutex);         /* so we sleep then wake up */
            /* fall through */
        case kStateReady:
            memory_barrier();
            return s->p;                     /* uncontended is lockfree */
        }
        old_state = compare_and_swap(&s->state, kStateNone, kStateBusy);
        if (old_state == kStateNone) {       /* None -> Busy */
            mutex_init(&s->mutex);
            mutex_lock(&s->mutex);
            s->state = kStateRunning;        /* Busy -> Running */
            memory_barrier();
            s->p = fn(arg);
            memory_barrier();
            s->state = kStateReady;          /* Running -> Ready */
            mutex_unlock(&s->mutex);
            return s->p;
        }
        if (old_state == kStateBusy) {
            do { old_state = s->state; }
            while (old_state == kStateBusy); /* Busy -> Busy */
        }
    } while (trys-- > 0);
    abort();
}

void* my_ctor(size_t sz) { void *p = malloc(sz); memset(p, 0x55, sz); return p; }

once_t s;

int main(int argc, char **argv)
{
    int *a, *b;
    const size_t sz = 8192 * sizeof(int);

    /* global reference to heap */
    a = (int *)init_once(&s, sz, my_ctor);
    assert(a);

    /* thread local reference to heap */
    b = (int*)malloc(sz);
    memset(b, 0x55, sz);
    assert(memcmp(a, b, sz) == 0);

    free(b);
    return 0;
}