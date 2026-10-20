/*
 * 06_mutex_not_shared_bug.c - a mutex in shared memory, not marked shared.
 *
 * Run:   ./06_mutex_not_shared_bug
 *        The fix is in 06_mutex_not_shared_fixed.c. To see what changed:
 *        diff 06_mutex_not_shared_bug.c 06_mutex_not_shared_fixed.c
 *
 * Look:  NPROC processes each run NTHREADS threads, and every thread adds
 *        to one counter in a shared memory object. The counter is protected
 *        by a mutex (see 04-threads/07_mutex.c) that also lives in the
 *        object. All threads of all processes see the same memory, so this
 *        looks right. Yet a process gets stuck, alarm(5) kills it, and the
 *        total comes out short.
 *
 *        By default a mutex is only for the threads of one process. When a
 *        thread waits for it, the kernel keeps the waiting thread in a list
 *        that belongs to its process. When a thread in the other process
 *        unlocks, the kernel looks only in that process's list, so the
 *        waiter is never woken.
 *
 *        Nothing reports an error: every lock and unlock succeeds.
 *
 *        The object is unlinked right after mapping. The children inherit
 *        the mapping through fork(), so they no longer need its name.
 */
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NAME     "/k22_shm_mutex"
#define NPROC    2
#define NTHREADS 2
#define COUNT    1000000        /* additions per thread */

struct shared {
    pthread_mutex_t lock;
    long            counter;
};

static struct shared *s;

static void *add(void *arg)
{
    (void) arg;
    for (long i = 0; i < COUNT; i++) {
        if ((errno = pthread_mutex_lock(&s->lock)) != 0)
            die("pthread_mutex_lock");
        s->counter++;                   /* critical section */
        if ((errno = pthread_mutex_unlock(&s->lock)) != 0)
            die("pthread_mutex_unlock");
    }
    return NULL;
}

int main(void)
{
    int fd = shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0)
        die("shm_open");
    if (shm_unlink(NAME) < 0)
        die("shm_unlink");
    if (ftruncate(fd, sizeof(struct shared)) < 0)
        die("ftruncate");
    s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (s == MAP_FAILED)
        die("mmap");
    close(fd);

    pthread_mutexattr_t attr;
    if ((errno = pthread_mutexattr_init(&attr)) != 0)
        die("pthread_mutexattr_init");
    /* BUG: the mutex is left private to one process */
    if ((errno = pthread_mutex_init(&s->lock, &attr)) != 0)
        die("pthread_mutex_init");
    s->counter = 0;

    setvbuf(stdout, NULL, _IOLBF, 0);
    for (int p = 0; p < NPROC; p++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            alarm(5);                   /* give up after 5 seconds */
            pthread_t t[NTHREADS];
            for (int i = 0; i < NTHREADS; i++)
                if ((errno = pthread_create(&t[i], NULL, add, NULL)) != 0)
                    die("pthread_create");
            for (int i = 0; i < NTHREADS; i++)
                if ((errno = pthread_join(t[i], NULL)) != 0)
                    die("pthread_join");
            return 0;
        }
    }

    int status;
    pid_t pid;
    while ((pid = wait(&status)) > 0)
        printf("process %d %s\n", (int) pid,
               WIFSIGNALED(status) ? "was killed by SIGALRM: stuck"
                                   : "finished");

    long expected = NPROC * NTHREADS * COUNT;
    printf("expected %ld, got %ld (%ld updates lost)\n",
           expected, s->counter, expected - s->counter);

    pthread_mutex_destroy(&s->lock);
    munmap(s, sizeof(*s));
    return 0;
}
