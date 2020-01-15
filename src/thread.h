#pragma once

#if defined (_WIN32)
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

#if defined (_WIN32)
inline long compare_and_swap(long volatile *t, long o, long n) {
    return InterlockedCompareExchange(t,n,o);
}
typedef unsigned thread_t;
typedef CONDITION_VARIABLE cond_t;
typedef CRITICAL_SECTION mutex_t;
inline int thread_create(thread_t *thread, unsigned (__stdcall *threadfunc) (void*), void* arg) {
    return (int)_beginthreadex(NULL, 0, threadfunc, arg, 0, thread);
}
#define thread_exit(rc) _endthreadex(rc)
inline void thread_join(thread_t *thread) { WaitForSingleObject( thread, INFINITE ); CloseHandle(&thread); }
inline void cond_init(cond_t *cond) { InitializeConditionVariable(cond); }
inline void cond_destroy(cond_t *cond) { }
inline void cond_wait(cond_t *cond, mutex_t *mutex) { SleepConditionVariableCS(cond, mutex, INFINITE); }
inline void cond_signal(cond_t *cond) { WakeConditionVariable(cond); }
inline void cond_broadcast(cond_t *cond) { WakeAllConditionVariable(cond); }
inline void memory_barrier() { MemoryBarrier(); }
inline void mutex_init(mutex_t *mutex) { InitializeCriticalSection(mutex); }
inline void mutex_destroy(mutex_t *mutex) { DeleteCriticalSection(mutex); }
inline void mutex_lock(mutex_t *mutex) { EnterCriticalSection(mutex); }
inline void mutex_unlock(mutex_t *mutex) { LeaveCriticalSection(mutex); }
#else
inline void memory_barrier() { asm volatile("": : :"memory"); }
inline long compare_and_swap(long volatile *t, long o, long n) {
    return __sync_val_compare_and_swap(t,o,n);
}
typedef pthread_t thread_t;
typedef pthread_cond_t cond_t;
typedef pthread_mutex_t mutex_t;
inline int thread_create(thread_t *thread, void* (*threadfunc)(void*), void* arg) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    return (int)pthread_create(thread, &attr, threadfunc, arg);
}
#define thread_exit(rc) do { pthread_exit((void*)rc); return NULL } while (0);
inline void thread_join(thread_t *thread) { void *result; pthread_join(*thread, &result); }
inline void cond_init(cond_t *cond) { pthread_cond_init(cond, NULL); }
inline void cond_destroy(cond_t *cond) { pthread_cond_destroy(cond); }
inline void cond_wait(cond_t *cond, mutex_t *mutex) { pthread_cond_wait(cond, mutex); }
inline void cond_signal(cond_t *cond) { pthread_cond_signal(cond); }
inline void cond_broadcast(cond_t *cond) { pthread_cond_broadcast(cond); }
inline void mutex_init(mutex_t *mutex) { pthread_mutex_init(mutex, NULL); }
inline void mutex_destroy(mutex_t *mutex) { pthread_mutex_destroy(mutex); }
inline void mutex_lock(mutex_t *mutex) { pthread_mutex_lock(mutex); }
inline void mutex_unlock(mutex_t *mutex) { pthread_mutex_unlock(mutex); }
#endif