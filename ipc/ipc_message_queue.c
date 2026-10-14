/*
 * IPC Mechanism (b): MESSAGE QUEUES
 * ---------------------------------------------------------------------
 * Demonstrates inter-process communication using a System V message
 * queue between a parent process and a child process created with
 * fork(). Unlike a pipe, a message queue is message-oriented (each
 * send/receive moves one discrete, typed message, not a raw byte
 * stream) and messages can optionally be filtered by "type" on the
 * receiving end.
 *
 * Here the PARENT sends two typed messages and the CHILD receives
 * them by type, in a different order than they were sent - showing
 * that a message queue lets a receiver pick a specific message type
 * instead of being forced to read in strict FIFO order like a pipe.
 *
 * Compile: gcc ipc_message_queue.c -o ipc_message_queue
 * Run    : ./ipc_message_queue
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_TEXT_SIZE 256
#define TYPE_URGENT   1
#define TYPE_NORMAL   2

struct message {
    long mtype;                   /* message type - must be a positive long */
    char mtext[MSG_TEXT_SIZE];    /* message payload */
};

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);  /* line-buffer stdout so parent/child
                                           output interleaves in real order even
                                           when run with output redirected */
    /* IPC_PRIVATE creates a brand-new queue just for this program run,
       so we don't need a key file on disk (see ftok(3) for named queues
       shared between unrelated programs). */
    int msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }
    printf("[Setup] Message queue created, id = %d\n", msgid);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid > 0) {
        /* ---------- PARENT process: sender ---------- */
        struct message msg;

        msg.mtype = TYPE_NORMAL;
        strcpy(msg.mtext, "This is a NORMAL-priority message.");
        msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);
        printf("[Parent, pid=%d] Sent (type=NORMAL):  \"%s\"\n", getpid(), msg.mtext);

        msg.mtype = TYPE_URGENT;
        strcpy(msg.mtext, "This is an URGENT message!");
        msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);
        printf("[Parent, pid=%d] Sent (type=URGENT):  \"%s\"\n", getpid(), msg.mtext);

        wait(NULL);                               /* wait for child to finish */

        msgctl(msgid, IPC_RMID, NULL);             /* remove the queue */
        printf("[Parent, pid=%d] Child finished; message queue removed.\n", getpid());

    } else {
        /* ---------- CHILD process: receiver ---------- */
        struct message msg;

        /* Even though URGENT was sent second, requesting TYPE_URGENT
           explicitly retrieves it first - demonstrating type-based,
           non-FIFO delivery that plain pipes cannot do. */
        msgrcv(msgid, &msg, MSG_TEXT_SIZE, TYPE_URGENT, 0);
        printf("[Child,  pid=%d] Received (type=URGENT): \"%s\"\n", getpid(), msg.mtext);

        msgrcv(msgid, &msg, MSG_TEXT_SIZE, TYPE_NORMAL, 0);
        printf("[Child,  pid=%d] Received (type=NORMAL): \"%s\"\n", getpid(), msg.mtext);
    }

    return 0;
}

/*
 * Sample output:
 *
 * [Setup] Message queue created, id = 0
 * [Parent, pid=12345] Sent (type=NORMAL):  "This is a NORMAL-priority message."
 * [Parent, pid=12345] Sent (type=URGENT):  "This is an URGENT message!"
 * [Child,  pid=12346] Received (type=URGENT): "This is an URGENT message!"
 * [Child,  pid=12346] Received (type=NORMAL): "This is a NORMAL-priority message."
 * [Parent, pid=12345] Child finished; message queue removed.
 *
 * Note: if the program is interrupted before cleanup, list and remove any
 * leftover queue manually with:  ipcs -q   and   ipcrm -q <msgid>
 */
