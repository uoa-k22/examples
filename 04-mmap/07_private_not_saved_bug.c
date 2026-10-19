/*
 * 07_private_not_saved_bug.c - writes through a MAP_PRIVATE file mapping are
 * lost.
 *
 * Run:   ./07_private_not_saved_bug          the bug
 *        ./07_private_not_saved_bug fixed    the fix
 *        cat private_demo.txt
 *
 * Look:  the program wants to change the first word of a file through a
 *        mapping, as 03_file_write.c does. The buggy version maps the file
 *        with MAP_PRIVATE. The store works, and reading the mapping shows the
 *        new text, but the write went to a private copy of the page
 *        (copy-on-write), not to the file. Even msync() cannot help: there
 *        is nothing to send. When the program reads the file back with
 *        pread(), the old text is still there, and it stays there after the
 *        program exits. No error is reported anywhere.
 *
 *        With MAP_SHARED the store changes the page cache that the file
 *        itself uses, so the file is changed.
 *
 *        MAP_PRIVATE on a file is useful for reading, or when you want a
 *        scratch copy that never reaches the disk. It is the wrong flag
 *        when the point is to save the changes.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "errexit.h"

#define FILENAME "private_demo.txt"

static void show_file(int fd, size_t len)
{
    char buf[64];
    size_t want = len < sizeof(buf) - 1 ? len : sizeof(buf) - 1;
    ssize_t n = pread(fd, buf, want, 0);
    if (n < 0)
        die("pread");
    buf[n] = '\0';
    printf("  file contains:    \"%s\"\n", buf);
}

int main(int argc, char *argv[])
{
    int fixed = argc > 1 && strcmp(argv[1], "fixed") == 0;

    const char *original = "old text";
    size_t len = strlen(original);

    int fd = open(FILENAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        die(FILENAME);
    if (write(fd, original, len) != (ssize_t) len)
        die("write");

    /* BUG: MAP_PRIVATE keeps the changes in this process only. */
    char *p = mmap(NULL, len, PROT_READ | PROT_WRITE,
                   fixed ? MAP_SHARED : MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED)
        die("mmap");

    printf("Mapped %s with %s\n",
           FILENAME, fixed ? "MAP_SHARED" : "MAP_PRIVATE");
    memcpy(p, "new", 3);
    if (msync(p, len, MS_SYNC) < 0)
        die("msync");

    printf("After storing \"new\" into the mapping:\n");
    printf("  mapping contains: \"%.*s\"\n", (int) len, p);
    show_file(fd, len);

    munmap(p, len);
    close(fd);
    return 0;
}
