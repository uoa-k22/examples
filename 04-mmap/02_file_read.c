/*
 * 02_file_read.c - reading a file through a memory mapping.
 *
 * Run:   ./02_file_read 02_file_read.c
 *
 * Look:  instead of read() into a buffer, the whole file is mapped into
 *        memory and used as an array of bytes. Pages are loaded from disk
 *        when they are first touched (page faults). The descriptor can be
 *        closed straight after mmap(); the mapping stays valid. An empty
 *        file cannot be mapped (length 0 is an error), so check for it.
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "errexit.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
        die(argv[1]);

    struct stat st;
    if (fstat(fd, &st) < 0)
        die("fstat");
    if (st.st_size == 0)
        return 0;

    char *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED)
        die("mmap");
    close(fd);

    /* Count the lines by reading the mapping like an ordinary array. */
    long lines = 0;
    for (off_t i = 0; i < st.st_size; i++)
        if (data[i] == '\n')
            lines++;

    /* write() may write less than asked, so loop until everything is out. */
    for (off_t done = 0; done < st.st_size; ) {
        ssize_t n = write(STDOUT_FILENO, data + done, st.st_size - done);
        if (n < 0)
            die("write");
        done += n;
    }
    fprintf(stderr, "--- %s: %ld bytes, %ld lines\n", argv[1], (long) st.st_size, lines);

    if (munmap(data, st.st_size) < 0)
        die("munmap");
    return 0;
}
