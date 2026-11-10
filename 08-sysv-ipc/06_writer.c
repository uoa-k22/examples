/*
 * 06_writer.c - create a System V segment with a key from ftok() and
 * write to it.
 *
 * Run:   ./06_writer "hello from the writer"
 *        ipcs -m
 *        ./07_reader
 *        (or just: make run)
 *
 * Look:  IPC_PRIVATE (01_sysv_shm) only works for a parent and its
 *        children, which inherit the shmid. Unrelated processes need a key
 *        they can both compute. ftok() makes one from a file that exists
 *        and a project id, so the writer and the reader print the same key.
 *        shmget() with IPC_CREAT creates the segment for that key, or
 *        returns the one that already exists.
 *
 *        The writer then exits, but the segment stays (ipcs -m shows the
 *        key and the shmid) until someone calls shmctl(IPC_RMID). Here the
 *        reader does that. Nothing here makes the reader wait for the
 *        writer; that needs a semaphore (see 03_sysv_sem).
 */
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include "errexit.h"
#include "sysv_common.h"

int main(int argc, char *argv[])
{
    const char *text = argc > 1 ? argv[1] : "hello through a System V key";

    key_t key = ftok(KEY_PATH, KEY_ID);
    if (key < 0)
        die("ftok " KEY_PATH);
    int shmid = shmget(key, sizeof(struct message), IPC_CREAT | 0600);
    if (shmid < 0)
        die("shmget");

    struct message *m = shmat(shmid, NULL, 0);
    if (m == (void *) -1)
        die("shmat");

    m->writer = getpid();
    snprintf(m->text, sizeof(m->text), "%s", text);
    printf("writer %d: key 0x%08x, segment %d: stored \"%s\"\n",
           (int) m->writer, (unsigned) key, shmid, m->text);

    shmdt(m);
    return 0;
}
