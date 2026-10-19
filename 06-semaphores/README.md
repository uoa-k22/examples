# 06 — Semaphores

A semaphore is a counter that never goes below zero. `sem_wait()` (P)
decrements it, blocking while it is 0. `sem_post()` (V) increments it,
waking a waiter. Semaphores are used in two ways:

- **mutual exclusion**: start at 1, and wrap a critical section in P … V
- **signalling**: start at 0; one process waits until another posts

| Program | Shows |
| --- | --- |
| `01_race_bug` / `_fixed` | lost updates when two processes increment a shared counter (run it several times: the number lost changes every time); the fix is an unnamed semaphore in shared memory |
| `03_named_sem_parent` | a named semaphore (`sem_open`) shared across `exec`; starts `03_named_sem_child` |
| `04_pingpong` | two semaphores enforce strict alternation |
| `05_bounded_buffer` | producer/consumer with `empty`, `full` and `mutex` |
| `06_sysv_sem` | the System V API: `semget` / `semctl` / `semop` with `SEM_UNDO` |

More mistakes. A hang ends by itself via `alarm()`. Each `_bug` program has
a `_fixed` partner with the mistake corrected; `diff` the two files to see
the fix.

| Program | Shows |
| --- | --- |
| `07_sem_not_shared_bug` / `_fixed` | a semaphore in a global variable is copied by `fork()`, so it does not protect anything; the fix puts it in shared memory |
| `08_deadlock_bug` / `_fixed` | two semaphores taken in opposite orders deadlock (ended after 5 s by `alarm()`); the fix takes them in the same order |
| `09_owner_died_bug` / `_fixed` | a process killed while holding a semaphore leaves it locked forever; the fix is the flag `SEM_UNDO` |
| `10_wrong_init_value_bug` / `_fixed` | a lock initialised to 0 instead of 1: everyone waits forever |
| `11_lock_leak_bug` / `_fixed` | an early return that skips `sem_post()`: a later process hangs |
| `12_producer_order_bug` / `_fixed` | the producer takes `mutex` before `empty`: the full buffer deadlocks |
| `13_eintr_bug` / `_fixed` | a signal handler makes `sem_wait()` fail with `EINTR`; the fix is the retry loop |

## About EINTR

A call that blocks, such as `sem_wait()`, `semop()` or `wait()`, can fail
with `errno == EINTR` ("Interrupted system call") when a signal handler runs
while it is waiting. This is not a real error, and the usual response is to
call it again:

```c
while (sem_wait(sem) < 0)
    if (errno != EINTR)
        die("sem_wait");
```

Once a program installs a signal handler, use this loop for every blocking
call; `13_eintr_bug` shows what happens without it. The other programs here
install no handlers, so they keep the simpler one-line check, with one
exception: on Linux, System V `semop()` also fails with `EINTR` when the
process is stopped (Ctrl-Z) and continued (`fg`), even without a handler, so
`06_sysv_sem` and `09_owner_died_bug`/`_fixed` retry it.

If a named-semaphore run is interrupted, remove the leftover with
`rm /dev/shm/sem.k22_named_sem`.
