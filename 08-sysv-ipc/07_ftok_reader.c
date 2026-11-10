/*
 * 07_ftok_reader.c - find an existing System V segment by its ftok() key.
 *
 * Run:   after 06_ftok_writer (see there), or: make run
 *
 * Look:  the reader calls ftok() with the same path and project id as the
 *        writer, so it gets the same key. shmget() without IPC_CREAT finds
 *        the segment for that key, and the reader attaches it read-only.
 *        It sees the message left by a process that has already exited.
 *        Finally it removes the segment with shmctl(IPC_RMID). Run it a
 *        second time and shmget() fails with "No such file or directory".
 */
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include "errexit.h"
#include "ftok_common.h"

int main(void)
{
    key_t key = ftok(KEY_PATH, KEY_ID);
    if (key < 0)
        die("ftok " KEY_PATH);
    int shmid = shmget(key, 0, 0);
    if (shmid < 0)
        die("shmget");

    struct message *m = shmat(shmid, NULL, SHM_RDONLY);
    if (m == (void *) -1)
        die("shmat");

    printf("reader %d: key 0x%08x, segment %d: process %d left the "
           "message \"%s\"\n", (int) getpid(), (unsigned) key, shmid,
           (int) m->writer, m->text);

    shmdt(m);
    if (shmctl(shmid, IPC_RMID, NULL) < 0)
        die("shmctl IPC_RMID");
    printf("reader %d: removed segment %d\n", (int) getpid(), shmid);
    return 0;
}
