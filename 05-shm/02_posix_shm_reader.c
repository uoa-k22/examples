/*
 * 02_posix_shm_reader.c - open an existing POSIX shared memory object.
 *
 * Run:   after 01_posix_shm_writer (see there), or: make run
 *
 * Look:  the reader opens the object by name, without O_CREAT, and maps it
 *        read-only. It sees the message left by a process that has already
 *        exited. Finally it calls shm_unlink() to remove the name. Run it a
 *        second time and shm_open() fails with "No such file or directory".
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "errexit.h"
#include "shm_common.h"

int main(void)
{
    int fd = shm_open(SHM_NAME, O_RDONLY, 0);
    if (fd < 0)
        die("shm_open " SHM_NAME);

    struct message *m = mmap(NULL, sizeof(*m), PROT_READ, MAP_SHARED, fd, 0);
    if (m == MAP_FAILED)
        die("mmap");
    close(fd);

    printf("Reader %d: process %d left the message \"%s\"\n",
           (int) getpid(), (int) m->writer, m->text);

    if (munmap(m, sizeof(*m)) < 0)
        die("munmap");
    if (shm_unlink(SHM_NAME) < 0)
        die("shm_unlink");
    printf("Reader %d: removed %s\n", (int) getpid(), SHM_NAME);
    return 0;
}
