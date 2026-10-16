/*
 * 03_file_write.c - writing a file through a shared memory mapping.
 *
 * Run:   ./03_file_write && cat mmap_demo.txt
 *
 * Look:  with MAP_SHARED, stores into the mapping change the file itself.
 *        A mapping cannot make a file longer, so the file is first given
 *        its final size with ftruncate(). Touching a page past the end of
 *        the file raises SIGBUS. msync() forces the changes to disk now;
 *        without it the kernel writes them back later.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "errexit.h"

#define FILENAME "mmap_demo.txt"

int main(void)
{
    const char *msg = "This text was written into memory, not with write().\n";
    size_t len = strlen(msg);

    int fd = open(FILENAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        die(FILENAME);
    if (ftruncate(fd, len) < 0)
        die("ftruncate");

    char *p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
        die("mmap");
    close(fd);

    memcpy(p, msg, len);
    if (msync(p, len, MS_SYNC) < 0)
        die("msync");
    if (munmap(p, len) < 0)
        die("munmap");

    printf("Wrote %zu bytes to %s through the mapping\n", len, FILENAME);
    return 0;
}
