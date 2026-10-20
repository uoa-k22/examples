# 06 — Shared memory

Shared memory lets processes read and write the same physical memory. It is
the fastest form of IPC, because no data is copied through the kernel. But
nothing coordinates the processes; that is the job of semaphores
(`07-semaphores`). This topic uses the POSIX API; the older System V API
is in `08-sysv-ipc`.

| Program | Shows |
| --- | --- |
| `01_posix_shm_writer` | `shm_open()` + `ftruncate()` + `mmap()`: create a named object and write to it |
| `02_posix_shm_reader` | open the object by name, read it, `shm_unlink()` it |
| `03_shm_pointers_bug` / `_fixed` | pointers stored in shared memory are wrong in another process; the fix links the nodes with indexes |
| `04_no_ftruncate_bug` / `_fixed` | mapping an object whose size is 0: the first store gives `SIGBUS`; the fix calls `ftruncate()` |
| `05_read_before_write_bug` / `_fixed` | the reader sees an empty buffer because nothing says the data is ready; the fix waits on a semaphore |

Each `_bug` program has a `_fixed` partner with the mistake corrected;
`diff` the two files to see the fix.

`make run` runs the writer, shows the object in `/dev/shm`, then runs the
reader.

## Leftovers

Shared memory objects outlive the processes that create them. If a demo is
interrupted, remove what it left behind:

| Program | Clean up with |
| --- | --- |
| `01_posix_shm_writer`, `02_posix_shm_reader` | `rm /dev/shm/k22_shm_demo` |
| `03_shm_pointers_bug` / `_fixed` | `rm /dev/shm/k22_shm_list` |
| `04_no_ftruncate_bug` / `_fixed` | `rm /dev/shm/k22_shm_noftruncate` |
| `05_read_before_write_bug` / `_fixed` | `rm /dev/shm/k22_shm_readwrite` |
