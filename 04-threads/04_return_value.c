/*
 * 04_return_value.c - threads return their results through pthread_join().
 *
 * Run:   ./04_return_value
 *
 * Look:  the numbers 1 .. 4000000 are split into 4 parts, and each thread
 *        adds up one part. A thread's function returns a void *, and
 *        pthread_join() stores it in the variable its second argument points
 *        to. Here the sum is put into the pointer itself, as the argument
 *        was in 02_many_threads.c. main() adds the 4 results and checks the
 *        total against the formula n(n + 1) / 2.
 *
 *        Returning from the thread's function ends the thread; calling
 *        pthread_exit(value) does the same from anywhere in the thread.
 */
#include <pthread.h>
#include <stdio.h>
#include "errexit.h"

#define NTHREADS 4
#define N        4000000L
#define PART     (N / NTHREADS)

static void *sum_part(void *arg)
{
    long first = (long) arg * PART + 1;
    long sum = 0;

    for (long i = first; i < first + PART; i++)
        sum += i;
    printf("thread %ld: %ld .. %ld adds up to %ld\n",
           (long) arg, first, first + PART - 1, sum);
    return (void *) sum;
}

int main(void)
{
    pthread_t t[NTHREADS];

    for (long i = 0; i < NTHREADS; i++)
        if ((errno = pthread_create(&t[i], NULL, sum_part, (void *) i)) != 0)
            die("pthread_create");

    long total = 0;
    for (int i = 0; i < NTHREADS; i++) {
        void *result;
        if ((errno = pthread_join(t[i], &result)) != 0)
            die("pthread_join");
        total += (long) result;
    }
    printf("main: total %ld, expected %ld\n", total, N * (N + 1) / 2);
    return 0;
}
