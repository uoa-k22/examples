# 01 — fork()

`fork()` creates a new process (the child) that is a copy of the calling
process (the parent). Both continue from the instruction after `fork()`.
The only difference is the return value: the child's pid in the parent and
`0` in the child.

| Program | Shows |
| --- | --- |
| `01_simple_fork` | the two return values of `fork()`; `getpid()` / `getppid()` |
| `02_address_space` | the child's changes to variables are not seen by the parent |
| `03_shared_fd` | an open file descriptor is shared, including its offset (`cat shared_fd.txt` afterwards) |
| `04_orphan` | a child whose parent has exited is adopted by init or a subreaper |
| `05_fork_loop` | the right way to fork in a loop; `./05_fork_loop bug` shows how 2^N processes appear |
| `06_stdio_buffer` | output printed before `fork()` appears twice when piped: comment out its `fflush()` and run `./06_stdio_buffer \| cat` |
| `07_separate_open_bug` / `_fixed` | two processes that `open()` the same file separately overwrite each other; the fix uses `O_APPEND` |
| `08_fork_limit_bug` / `_fixed` | `fork()` fails with `EAGAIN` and an unchecked `-1` looks like success; the fix checks it |

Each `_bug` program has a `_fixed` partner with the mistake corrected;
`diff` the two files to see the fix.

Build with `make` and run each program from this directory.
