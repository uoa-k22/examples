# K22 Operating Systems — example programs

Small C programs that accompany the K22 Operating Systems course. Each
program shows one idea about processes or interprocess communication on
Linux/POSIX.

## Topics

| Directory | Topic |
|---|---|
| [`01-fork`](01-fork) | creating processes with `fork()` |
| [`02-wait`](02-wait) | `wait()` / `waitpid()`, exit status, zombies and orphans |
| [`03-exec`](03-exec) | running a new program with the `exec` family; fork + exec + wait |
| [`04-mmap`](04-mmap) | memory mappings: anonymous and file-backed, private and shared |
| [`05-shm`](05-shm) | POSIX (`shm_open`) and System V (`shmget`) shared memory |
| [`06-semaphores`](06-semaphores) | race conditions, mutual exclusion and signalling with semaphores |

Work through them in order; later topics use earlier ones.

## Building and running

You need `gcc` and `make` on Linux.

```sh
make                        # build everything
make EXTRA_CFLAGS=-Werror   # build, treating warnings as errors
make clean                  # remove binaries and files the demos create
```

Or build one topic with `make` inside its directory. Each `.c` file is its
own program: `01_simple_fork.c` builds `./01_simple_fork`. Run programs from
their own directory.

## How to read the examples

- The comment at the top of each file explains what to run, what to look
  for in the output, and things to try.
- Every system call is checked. `die("name")` from
  [`common/errexit.h`](common/errexit.h) prints the error and exits, so the
  check takes one line.
- Output from processes that run at the same time can come in a different
  order on every run. That is expected, and often it is the point.
