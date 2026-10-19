# 03 — exec

The `exec` family replaces the program running in a process with a new one.
The pid stays the same, and open file descriptors stay open. `exec` returns
only if it fails, so the line after it is always error handling.

| Name | Arguments as | Searches `$PATH` | Environment |
| --- | --- | --- | --- |
| `execv` / `execl` | array / list | no | inherited |
| `execvp` / `execlp` | array / list | yes | inherited |
| `execve` / `execle` | array / list | no | given explicitly |

| Program | Shows |
| --- | --- |
| `01_execv_path` | `execv` needs a path; code after a successful exec never runs |
| `02_execvp` | `$PATH` lookup; `argv[0]` is only a convention; `./02_execvp date -u` |
| `03_fork_exec_wait` | fork + exec + wait, the core of a shell: `./03_fork_exec_wait ls -l` |
| `04_env` | `execle` with a custom environment |
| `05_exec_no_exit_bug` / `_fixed` | a child that keeps running the parent's code after a failed exec, so the failure looks like success; the fix adds `_exit(127)` |
| `06_fd_inherit` | open file descriptors are inherited across exec; `O_CLOEXEC` closes them |
| `07_self_exec_bug` / `_fixed` | a program that execs itself through `argv[0]` fails after `chdir()`; the fix uses `/proc/self/exe` |
| `08_exec_errors_bug` / `_fixed` | `ENOENT`, `EACCES` and `ENOEXEC` are three different failures; the fix corrects each |

Each `_bug` program has a `_fixed` partner with the mistake corrected;
`diff` the two files to see the fix.

Build with `make` and run each program from this directory.
