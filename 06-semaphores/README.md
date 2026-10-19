# 06 — Semaphores

A semaphore is a counter that never goes below zero. `sem_wait()` (P)
decrements it, blocking while it is 0. `sem_post()` (V) increments it,
waking a waiter. Semaphores are used in two ways:

- **mutual exclusion**: start at 1, and wrap a critical section in P … V
- **signalling**: start at 0; one process waits until another posts

| Program | Shows |
|---|---|
| `01_race` | lost updates when two processes increment a shared counter |
| `02_unnamed_sem` | the same counter protected by an unnamed semaphore in shared memory |
| `03_named_sem_parent` | a named semaphore (`sem_open`) shared across `exec`; starts `03_named_sem_child` |
| `04_pingpong` | two semaphores enforce strict alternation |
| `05_bounded_buffer` | producer/consumer with `empty`, `full` and `mutex` |
| `06_sysv_sem` | the System V API: `semget` / `semctl` / `semop` with `SEM_UNDO` |

Common mistakes. Each program works correctly as written; its header
comment says what to change to see the bug: a line to uncomment (`07`), two
lines to swap (`08`), or an argument to pass (`09`).

| Program | Shows |
|---|---|
| `07_sem_not_shared` | a semaphore in a global variable is copied by `fork()`, so it does not protect anything |
| `08_deadlock` | two semaphores taken in opposite orders; ends after 5 s via `alarm()` |
| `09_owner_died` | a process killed while holding a POSIX semaphore leaves it locked forever; `sysv` shows `SEM_UNDO` recovering |
| `10_wrong_init_value_bug` | a lock initialised to 0 instead of 1: everyone waits forever |
| `11_lock_leak_bug` | an early return that skips `sem_post()`: a later process hangs |
| `12_producer_order_bug` | the producer takes `mutex` before `empty`: the full buffer deadlocks |
| `13_eintr_bug` | a signal handler makes `sem_wait()` fail with `EINTR`; the retry loop fixes it |

`10` to `13` are the other way round: they fail as written (a hang ends by
itself via `alarm()`), and the argument `fixed` runs the correct version.

Build with `make` and run each program from this directory. Run `01_race`
several times. The number of lost updates changes from run to run; that is
the race.

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

These programs install no signal handlers, so `sem_wait()` and `wait()`
never fail this way here, and the examples keep the simpler one-line check.
The exception is System V `semop()`. On Linux it also fails with `EINTR`
when the process is stopped (Ctrl-Z) and then continued (`fg`), even
without a handler, so `06_sysv_sem` and `09_owner_died` retry it. Once you
add signal handlers (or `alarm()` with a handler) to a program, use the loop
above for every blocking call. `13_eintr_bug` shows the failure.

If a named-semaphore run is interrupted, remove the leftover with
`rm /dev/shm/sem.k22_named_sem`.
