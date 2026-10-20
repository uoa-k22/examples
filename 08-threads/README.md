# 08 — Threads

A thread is a flow of control inside a process. A process starts with one
thread, which runs `main()`; `pthread_create()` starts more. All threads of
a process share its memory (global variables, heap) and its open files.
Each thread has only its own stack and registers.

- `pthread_create()` starts a thread that runs a given function with one
  `void *` argument.
- `pthread_join()` waits for a thread to end and collects the `void *` its
  function returned, as `wait()` does for a child process.
- A `pthread_mutex_t` is a lock for the threads of one process:
  `pthread_mutex_lock()` / `pthread_mutex_unlock()`.

| Program | Shows |
| --- | --- |
| `01_create_join` | `pthread_create()` and `pthread_join()`; the thread runs in the same process (same pid) |
| `02_many_threads` | several threads, each with its own argument; same pid, different tid; the order changes on every run |
| `03_shared_globals` | a thread's changes to variables are seen by `main()`; compare `01-fork/02_address_space` |
| `04_return_value` | threads add up parts of a range and return their sums through `pthread_join()` |
| `05_shared_arg_bug` / `_fixed` | every thread gets a pointer to the same variable and reads the wrong number; the fix gives each its own |
| `06_race` | lost updates when two threads increment a shared counter, as in `06-semaphores/01_race` |
| `07_mutex` | the same counter protected by a mutex |
| `08_no_join_bug` / `_fixed` | `main()` returns without joining, and the threads die with the process; the fix joins them |
| `09_join_in_loop_bug` / `_fixed` | joining each thread right after starting it makes them run one at a time; the fix joins after starting all |

Each `_bug` program has a `_fixed` partner with the mistake corrected;
`diff` the two files to see the fix.

## Checking errors

The `pthread_*` functions do not set `errno`. They return 0 on success or
the error number on failure. The programs store that number in `errno`, so
that `die()` reports it like any other error:

```c
if ((errno = pthread_create(&t, NULL, worker, NULL)) != 0)
    die("pthread_create");
```

## Compiling

Link with `-pthread`, as `common/common.mk` does:
`gcc -pthread 01_create_join.c -o 01_create_join`.
