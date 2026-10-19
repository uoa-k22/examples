# 02 — wait() and waitpid()

A parent calls `wait()` or `waitpid()` to find out when a child has
terminated and how. Until the parent does so, the finished child stays a
*zombie*.

| Program | Shows |
| --- | --- |
| `01_wait_no_children` | `wait()` returns -1 with `errno == ECHILD` when there is nothing to wait for |
| `02_exit_status` | `WIFEXITED` / `WEXITSTATUS`; try `./02_exit_status 256` |
| `03_killed_by_signal` | `WIFSIGNALED` / `WTERMSIG` for children killed by `SIGSEGV`, `SIGABRT` and `SIGTERM` |
| `04_zombie` | the `Z` state in `ps` before the parent waits |
| `05_wait_order` | `wait()` returns children in the order they finish |
| `06_waitpid_order` | `waitpid()` collects the same children in the order the parent chooses; `diff` it with `05_wait_order.c` |
| `07_wnohang` | `waitpid(..., WNOHANG)`: check on a child without blocking |
| `08_wexitstatus_unchecked_bug` / `_fixed` | a crashed child reported as success because `WIFEXITED` was not checked; the fix checks it first |

Each `_bug` program has a `_fixed` partner with the mistake corrected;
`diff` the two files to see the fix.
