# 02 — wait() and waitpid()

A parent calls `wait()` or `waitpid()` to find out when a child has
terminated and how. Until the parent does so, the finished child stays a
*zombie*.

| Program | Shows |
|---|---|
| `01_wait_no_children` | `wait()` returns -1 with `errno == ECHILD` when there is nothing to wait for |
| `02_exit_status` | `WIFEXITED` / `WEXITSTATUS`; try `./02_exit_status 256` |
| `03_killed_by_signal` | `WIFSIGNALED` / `WTERMSIG` for `segv`, `abort`, or a child you `kill` |
| `04_zombie` | the `Z` state in `ps` before the parent waits |
| `05_multiple_children` | `wait()` returns in finishing order; `waitpid()` in the order you choose (`ordered`) |
| `06_wnohang` | `waitpid(..., WNOHANG)`: check on a child without blocking |

Build with `make` and run each program from this directory.
