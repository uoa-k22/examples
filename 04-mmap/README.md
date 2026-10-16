# 04 — mmap()

`mmap()` adds a region to the process's virtual address space. The region
can be backed by a file or by nothing (anonymous memory), and it can be
private to the process or shared with others.

|  | `MAP_PRIVATE` | `MAP_SHARED` |
|---|---|---|
| **file** | read a file as memory; writes stay in this process | writes go to the file |
| **anonymous** | plain memory (like `malloc`) | memory shared with children after `fork()` |

| Program | Shows |
|---|---|
| `01_anon_alloc` | anonymous private memory; page-sized allocation; matching `munmap` |
| `02_file_read` | mapping a file read-only: `./02_file_read 02_file_read.c` |
| `03_file_write` | writing a file through `MAP_SHARED`; `ftruncate` first, `msync` after |
| `04_shared_anon_fork` | shared vs. private anonymous memory across `fork()` (`private`) |

Build with `make` and run each program from this directory.
