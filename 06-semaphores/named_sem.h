/*
 * named_sem.h - settings shared by 02_named_sem_parent and 02_named_sem_child.
 */
#ifndef NAMED_SEM_H
#define NAMED_SEM_H

/* On Linux a named semaphore appears as /dev/shm/sem.k22_named_sem */
#define SEM_NAME    "/k22_named_sem"
#define CHILD_PROG  "./02_named_sem_child"
#define NCHILD      3
#define ROUNDS      3

#endif
