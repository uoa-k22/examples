/*
 * 07_private_not_saved_fixed.c - the fix for 07_private_not_saved_bug.c.
 *
 * Run:   ./07_private_not_saved_fixed
 *        cat private_demo.txt
 *        diff 07_private_not_saved_bug.c 07_private_not_saved_fixed.c
 *
 * Look:  the only change is MAP_SHARED instead of MAP_PRIVATE. Now the
 *        store changes the page cache that the file itself uses, so pread()
 *        and cat both show "new text". msync() makes sure the change has
 *        reached the disk before the program goes on.
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

int main(void)
{
    const char *original = "old text";
    size_t len = strlen(original);

    int fd = open(FILENAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        die(FILENAME);
    if (write(fd, original, len) != (ssize_t) len)
        die("write");

    /* FIX: MAP_SHARED writes go to the file. */
    char *p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
        die("mmap");

    printf("Mapped %s with %s\n", FILENAME, "MAP_SHARED");
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
