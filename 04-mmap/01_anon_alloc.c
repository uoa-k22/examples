/*
 * 01_anon_alloc.c - mmap() as a memory allocator.
 *
 * Run:   ./01_anon_alloc
 *
 * Look:  MAP_ANONYMOUS asks for memory that is not backed by any file, and
 *        filled with zeros. MAP_PRIVATE means it belongs to this process
 *        only. The kernel hands out whole pages, so a request for 5 ints
 *        really reserves one page. malloc() itself uses mmap() like this
 *        for large blocks. munmap() must be given the same length that was
 *        mapped.
 */
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "errexit.h"

#define N 5

int main(void)
{
    size_t len = N * sizeof(int);

    int *a = mmap(NULL, len, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (a == MAP_FAILED)
        die("mmap");

    printf("Mapped %zu bytes at %p (page size is %ld bytes)\n",
           len, (void *) a, sysconf(_SC_PAGESIZE));
    printf("Initial contents:");
    for (int i = 0; i < N; i++)
        printf(" %d", a[i]);
    printf("\n");

    for (int i = 0; i < N; i++)
        a[i] = 10 * i;
    printf("After filling:   ");
    for (int i = 0; i < N; i++)
        printf(" %d", a[i]);
    printf("\n");

    if (munmap(a, len) < 0)
        die("munmap");
    return 0;
}
