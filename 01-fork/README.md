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
| `05_fork_loop_bug` / `_fixed` | a child that does not exit inside a `fork()` loop: 2^N processes appear; the fix returns from the child |
| `06_stdio_buffer_bug` / `_fixed` | output printed before `fork()` appears twice when piped (`./06_stdio_buffer_bug \| cat`); the fix calls `fflush()` first |
| `07_separate_open_bug` / `_fixed` | two processes that `open()` the same file separately overwrite each other; the fix uses `O_APPEND` |
| `08_fork_limit_bug` / `_fixed` | `fork()` fails with `EAGAIN` and an unchecked `-1` looks like success; the fix checks it |

Each `_bug` program has a `_fixed` partner with the mistake corrected;
`diff` the two files to see the fix.
