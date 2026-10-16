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

Build with `make` and run each program from this directory. Run `01_race`
several times. The number of lost updates changes from run to run; that is
the race.

If a named-semaphore run is interrupted, remove the leftover with
`rm /dev/shm/sem.k22_named_sem`.
