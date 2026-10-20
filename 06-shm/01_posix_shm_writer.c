/*
 * 01_posix_shm_writer.c - create a POSIX shared memory object and write to it.
 *
 * Run:   ./01_posix_shm_writer "hello from the writer"
 *        ls -l /dev/shm/k22_shm_demo
 *        ./02_posix_shm_reader
 *        (or just: make run)
 *
 * Look:  shm_open() creates a named object, much like open() creates a file.
 *        It starts with size 0, so ftruncate() gives it its size, and mmap()
 *        maps it into memory. The writer then exits, but the object stays
 *        until someone calls shm_unlink(). Here the reader does that.
 *
 *        Unrelated processes can share memory this way, because they only
 *        need the name. Nothing here makes the reader wait for the writer;
 *        that needs a semaphore (see 07-semaphores).
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "errexit.h"
#include "shm_common.h"

int main(int argc, char *argv[])
{
    const char *text = argc > 1 ? argv[1] : "hello through shared memory";

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd < 0)
        die("shm_open");
    if (ftruncate(fd, sizeof(struct message)) < 0)
        die("ftruncate");

    struct message *m = mmap(NULL, sizeof(*m), PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd, 0);
    if (m == MAP_FAILED)
        die("mmap");
    close(fd);

    m->writer = getpid();
    snprintf(m->text, sizeof(m->text), "%s", text);
    printf("writer %d: stored \"%s\" in %s\n",
           (int) m->writer, m->text, SHM_NAME);

    if (munmap(m, sizeof(*m)) < 0)
        die("munmap");
    return 0;
}
