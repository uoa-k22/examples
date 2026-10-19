# 05 — Shared memory

Shared memory lets processes read and write the same physical memory. It is
the fastest form of IPC, because no data is copied through the kernel. But
nothing coordinates the processes; that is the job of semaphores
(`06-semaphores`). This topic uses the POSIX API; the older System V API
is in `07-sysv-ipc`.

| Program | Shows |
| --- | --- |
| `01_posix_shm_writer` | `shm_open` + `ftruncate` + `mmap`: create a named object and write to it |
| `02_posix_shm_reader` | open the object by name, read it, `shm_unlink` it |
| `04_shm_pointers_bug` / `_fixed` | pointers stored in shared memory are wrong in another process; the fix links the nodes with indexes |
| `05_no_ftruncate_bug` / `_fixed` | mapping an object whose size is 0: the first store gives SIGBUS; the fix calls `ftruncate` |
| `06_read_before_write_bug` / `_fixed` | the reader sees an empty buffer because nothing says the data is ready; the fix waits on a semaphore |

Each `_bug` program has a `_fixed` partner with the mistake corrected;
`diff` the two files to see the fix.

`make run` runs the writer, shows the object in `/dev/shm`, then runs the
reader.

Shared memory objects outlive the processes that create them. If a demo is
interrupted, clean up with `rm /dev/shm/k22_shm_demo` (`01`, `02`),
`rm /dev/shm/k22_shm_list` (`04`) or `rm /dev/shm/k22_shm_noftruncate`
(`05`).
