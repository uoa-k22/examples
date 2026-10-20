# 08 — System V IPC

System V IPC is the older API for shared memory and semaphores. It does the
same jobs as the POSIX calls in `06-shm` and `07-semaphores`, and many
existing programs still use it. The main differences:

- An object is identified by a number (`shmid`, `semid`), not a name.
- An object lives until it is removed with `IPC_RMID`, even after every
  process that used it has ended.
- `SEM_UNDO` lets the kernel release a semaphore held by a process that
  died. POSIX semaphores have no equivalent.

| Program | Shows |
| --- | --- |
| `01_sysv_shm` | shared memory: `shmget()` / `shmat()` / `shmdt()` / `shmctl(IPC_RMID)` |
| `02_leaked_segment_bug` / `_fixed` | a segment that is never removed with `IPC_RMID` outlives the program (`ipcs -m`); the fix removes it |
| `03_sysv_sem` | semaphores: `semget()` / `semctl()` / `semop()` with `SEM_UNDO` |
| `04_owner_died_bug` / `_fixed` | a process killed while holding a semaphore leaves it locked forever; the fix is the flag `SEM_UNDO` |

Each `_bug` program has a `_fixed` partner with the mistake corrected;
`diff` the two files to see the fix.

## Leftovers

`ipcs -m` lists the shared memory segments and `ipcs -s` the semaphore
sets. Remove one with `ipcrm -m <shmid>` or `ipcrm -s <semid>`.

`02_leaked_segment_bug` leaves a segment behind on purpose; it prints the
`ipcrm` command to remove it.

## EINTR and semop()

On Linux, a waiting `semop()` fails with `errno == EINTR` when the process
is stopped (Ctrl-Z) and continued (`fg`), even without a signal handler.
So `03_sysv_sem` and `04_owner_died_*` retry it. See "About EINTR" in
`07-semaphores` for the loop.
