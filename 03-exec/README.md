# 03 — exec

The `exec` family replaces the program running in a process with a new one.
The pid stays the same, and open file descriptors stay open. `exec` returns
only if it fails, so the line after it is always error handling.

| Name | Arguments as | Searches `$PATH` | Environment |
|---|---|---|---|
| `execv` / `execl` | array / list | no | inherited |
| `execvp` / `execlp` | array / list | yes | inherited |
| `execve` / `execle` | array / list | no | given explicitly |

| Program | Shows |
|---|---|
| `01_execv_path` | `execv` needs a path; code after a successful exec never runs |
| `02_execvp` | `$PATH` lookup; `argv[0]` is only a convention; `./02_execvp date -u` |
| `03_fork_exec_wait` | fork + exec + wait, the core of a shell: `./03_fork_exec_wait ls -l` |
| `04_exec_fail_bug` | a child that keeps running after a failed exec; `fixed` shows the fix |
| `05_env` | `execle` with a custom environment |

Build with `make` and run each program from this directory.
