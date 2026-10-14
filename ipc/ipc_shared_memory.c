/*
 * IPC Mechanism (c): SHARED MEMORY
 * ---------------------------------------------------------------------
 * Demonstrates inter-process communication using a System V shared
 * memory segment between a parent process and a child process created
 * with fork(). Shared memory is the fastest IPC mechanism because data
 * is NOT copied through the kernel on every read/write (as it is with
 * a pipe or a message queue) - both processes map the same physical
 * memory pages directly into their own address space.
 *
 * That speed comes with a catch: shared memory itself does nothing to
 * stop one process from reading before the other has finished writing.
 * Pipes and message queues block automatically until data is available;
 * plain shared memory does not. So this example places a small POSIX
 * semaphore INSIDE the shared segment purely as a "data is ready" flag,
 * to show why shared memory is normally paired with a separate
 * synchronization primitive.
 *
 * Compile: gcc ipc_shared_memory.c -o ipc_shared_memory -lpthread
 * Run    : ./ipc_shared_memory
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MSG_SIZE 256

typedef struct {
    sem_t ready;              /* signals "the message below has been written" */
    char  message[MSG_SIZE];  /* the actual shared payload */
} SharedData;

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);  /* line-buffer stdout so parent/child
                                           output interleaves in real order even
                                           when run with output redirected */
    /* IPC_PRIVATE creates a brand-new segment just for this program run
       (see ftok(3) if two unrelated programs need to agree on a key). */
    int shmid = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    printf("[Setup] Shared memory segment created, id = %d, size = %zu bytes\n",
           shmid, sizeof(SharedData));

    SharedData *shm = (SharedData *) shmat(shmid, NULL, 0);
    if (shm == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    /* pshared=1 -> the semaphore will be used between processes, not just threads */
    sem_init(&shm->ready, 1, 0);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid > 0) {
        /* ---------- PARENT process: writer ---------- */
        strcpy(shm->message, "Hello from the PARENT process via shared memory!");
        printf("[Parent, pid=%d] Wrote to shared memory: \"%s\"\n", getpid(), shm->message);

        sem_post(&shm->ready);                 /* tell the child the data is ready */

        wait(NULL);                            /* wait for child to finish reading */

        sem_destroy(&shm->ready);
        shmdt(shm);                            /* detach */
        shmctl(shmid, IPC_RMID, NULL);          /* remove the segment */
        printf("[Parent, pid=%d] Child finished; shared memory removed.\n", getpid());

    } else {
        /* ---------- CHILD process: reader ---------- */
        sem_wait(&shm->ready);                 /* block until parent has written */

        printf("[Child,  pid=%d] Read from shared memory: \"%s\"\n", getpid(), shm->message);

        shmdt(shm);                            /* detach (parent still owns/removes it) */
    }

    return 0;
}

/*
 * Sample output:
 *
 * [Setup] Shared memory segment created, id = 0, size = 288 bytes
 * [Parent, pid=12345] Wrote to shared memory: "Hello from the PARENT process via shared memory!"
 * [Child,  pid=12346] Read from shared memory: "Hello from the PARENT process via shared memory!"
 * [Parent, pid=12345] Child finished; shared memory removed.
 *
 * Note: if the program is interrupted before cleanup, list and remove any
 * leftover segment manually with:  ipcs -m   and   ipcrm -m <shmid>
 */
