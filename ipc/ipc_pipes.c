/*
 * IPC Mechanism (a): PIPES
 * ---------------------------------------------------------------------
 * Demonstrates inter-process communication using an unnamed pipe
 * between a parent process and a child process created with fork().
 *
 * A pipe is a unidirectional, byte-stream channel: whatever is written
 * to the "write end" of the pipe can be read, in order, from the
 * "read end". Here the PARENT writes a message and the CHILD reads it:
 *
 *      parent --write--> [ pipe buffer ] --read--> child
 *
 * Because the pipe only goes one way, the child closes the write end
 * it inherited from fork() (and doesn't use it), and the parent closes
 * the read end it doesn't need. This is the standard pipe idiom.
 *
 * Compile: gcc ipc_pipes.c -o ipc_pipes
 * Run    : ./ipc_pipes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MSG_SIZE 256

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);  /* line-buffer stdout so parent/child
                                           output interleaves in real order even
                                           when run with output redirected */
    int fd[2];              /* fd[0] = read end, fd[1] = write end */
    char write_msg[MSG_SIZE] = "Hello from the PARENT process via a pipe!";
    char read_msg[MSG_SIZE];

    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid > 0) {
        /* ---------- PARENT process: writer ---------- */
        close(fd[0]);                              /* don't need the read end */

        printf("[Parent, pid=%d] Sending message: \"%s\"\n", getpid(), write_msg);
        write(fd[1], write_msg, strlen(write_msg) + 1);
        close(fd[1]);                              /* signals EOF to the reader */

        wait(NULL);                                /* wait for child to finish */
        printf("[Parent, pid=%d] Child has finished. Exiting.\n", getpid());

    } else {
        /* ---------- CHILD process: reader ---------- */
        close(fd[1]);                              /* don't need the write end */

        int n = read(fd[0], read_msg, MSG_SIZE);
        if (n > 0) {
            printf("[Child,  pid=%d] Received message: \"%s\"\n", getpid(), read_msg);
        }
        close(fd[0]);
    }

    return 0;
}

/*
 * Sample output:
 *
 * [Parent, pid=12345] Sending message: "Hello from the PARENT process via a pipe!"
 * [Child,  pid=12346] Received message: "Hello from the PARENT process via a pipe!"
 * [Parent, pid=12345] Child has finished. Exiting.
 */
