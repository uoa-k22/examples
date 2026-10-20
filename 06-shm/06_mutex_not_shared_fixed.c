/*
 * 06_mutex_not_shared_fixed.c - the fix for 06_mutex_not_shared_bug.c.
 *
 * Run:   ./06_mutex_not_shared_fixed
 *        diff 06_mutex_not_shared_bug.c 06_mutex_not_shared_fixed.c
 *
 * Look:  pthread_mutexattr_setpshared() with PTHREAD_PROCESS_SHARED marks
 *        the mutex as shared by processes. The kernel then files a waiting
 *        thread under the shared memory itself, so an unlock in any process
 *        wakes it. Both processes finish and no update is lost.
 *
 *        A semaphore has the same choice: the pshared argument of
 *        sem_init() (see 07-semaphores/02_unnamed_sem.c).
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
    /* FIX: let threads of different processes share the mutex */
    if ((errno = pthread_mutexattr_setpshared(&attr,
                                              PTHREAD_PROCESS_SHARED)) != 0)
        die("pthread_mutexattr_setpshared");
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

    long expected = (long) NPROC * NTHREADS * COUNT;
    printf("expected %ld, got %ld (%ld updates lost)\n",
           expected, s->counter, expected - s->counter);

    pthread_mutex_destroy(&s->lock);
    munmap(s, sizeof(*s));
    return 0;
}
