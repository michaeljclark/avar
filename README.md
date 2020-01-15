# avar

How to initialize a global variable using a thread-safe initializer in C11.

## Overview

`avar` supports cross-platform thread-safe static initialization in C11.
C++11 supports thread-safe initialization of non-trivial global varaibles,
meaning global variables that are initialized by running code. This is
not easily possible in C11. The key problem is the necessity to prevent
threads from accessing global state during initialization.

These are the requirements for C initialization support:

- Initialization function should be run only once.
- Should not have any unexpected race conditions.
- Should be lock-free, using only atomic operations post initialization.

Safe static initialization that is thread-safe and cross-platform is hard.
The problems are manifold:

- No standard mutex types between POSIX and Windows.
- `<stdatomic.h>` is not supported in Microsoft Visual Studio.
- `pthread_init_once` is broken or has subtle bugs on Linux.
- Windows Critical Sections themself require initialization.
  - Problematic, as requires spinning until sleeping primitive is ready.
- Needs severalcross-platform locking and atomics primitives:
  - `mutex_lock`, `mutex_unlock`, `compare_and_swap` and `memory_barrier`.

`avar` is a condensation of the minimum dependencies needed to perform
thread-safe singleton initialization in C on POSIX and Windows. `avar`
provides thread-safe singleton initialization which is _lock-free_ in
the uncontended case, meaning it uses atomic memory operations.

## Build

Building on Windows

```
git clone https://github.com/michaeljclark/avar.git
cd avar
if not exist build mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release -j
```

Building on Linux

```
git clone https://github.com/michaeljclark/avar.git
cd avar
test -d build || mkdir build
cd build
cmake -G "Unix Makefiles" ..
cmake --build . --config Release -j
```
