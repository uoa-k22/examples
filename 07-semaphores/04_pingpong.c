/*
 * 04_pingpong.c - using semaphores to signal, not to lock.
 *
 * Run:   ./04_pingpong
 *
 * Look:  two semaphores pass a turn back and forth. "ping" starts at 1 and
 *        "pong" at 0, so the parent goes first. Each side waits on its own
 *        semaphore and posts the other's. The output therefore alternates
 *        strictly: ping, pong, ping, pong. Without the semaphores the two
 *        processes would print in whatever order the scheduler picked.
 *
 *        Here one process posts and a different one waits. A semaphore that
 *        starts at 0 means "wait until something has happened".
 */
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define ROUNDS 5

struct shared {
    sem_t ping;
    sem_t pong;
};

static void take_turns(sem_t *mine, sem_t *other, const char *word)
{
    for (int i = 1; i <= ROUNDS; i++) {
        if (sem_wait(mine) < 0)
            die("sem_wait");
        printf("%d: %s (pid %d)\n", i, word, (int) getpid());
        if (sem_post(other) < 0)
            die("sem_post");
    }
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);       /* print each line immediately */

    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    if (sem_init(&s->ping, 1, 1) < 0 || sem_init(&s->pong, 1, 0) < 0)
        die("sem_init");

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        take_turns(&s->pong, &s->ping, "    pong");
        return 0;
    }

    take_turns(&s->ping, &s->pong, "ping");
    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");

    sem_destroy(&s->ping);
    sem_destroy(&s->pong);
    munmap(s, sizeof(*s));
    return 0;
}
