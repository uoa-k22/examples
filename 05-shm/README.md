# 05 — Shared memory

Shared memory lets processes read and write the same physical memory. It is
the fastest form of IPC, because no data is copied through the kernel. But
nothing coordinates the processes; that is the job of semaphores
(`06-semaphores`).

| Program | Shows |
| --- | --- |
| `01_posix_shm_writer` | `shm_open` + `ftruncate` + `mmap`: create a named object and write to it |
| `02_posix_shm_reader` | open the object by name, read it, `shm_unlink` it |
| `03_sysv_shm` | the System V API: `shmget` / `shmat` / `shmdt` / `shmctl(IPC_RMID)` |
| `04_shm_pointers` | pointers stored in shared memory are wrong in another process; use indexes: uncomment its `use_pointers = 1` |
| `05_no_ftruncate_bug` | mapping an object whose size is 0: the first store gives SIGBUS; `fixed` calls `ftruncate` |
| `06_read_before_write_bug` | the reader sees an empty buffer because nothing says the data is ready; `fixed` adds a semaphore |
| `07_leaked_segment_bug` | a System V segment that is never removed with `IPC_RMID` outlives the program (`ipcs -m`); `fixed` removes it |

`05` to `07` fail as written (their names end in `_bug`); run them with
`fixed` to see the correct version.

`make run` runs the writer, shows the object in `/dev/shm`, then runs the
reader.

Shared memory objects outlive the processes that create them. If a demo is
interrupted, clean up with `rm /dev/shm/k22_shm_demo` (`01`/`02`),
`rm /dev/shm/k22_shm_list` (`04`) or `rm /dev/shm/k22_shm_noftruncate`
(`05`) for POSIX, or `ipcs -m` /
`ipcrm -m <shmid>` for System V. `07_leaked_segment_bug` leaves a System V
segment behind on purpose; it prints the `ipcrm` command to remove it.
