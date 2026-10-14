/*
 * UNIX/LINUX SYSTEM CALLS: fork(), exit(), wait()
 * ---------------------------------------------------------------------------------
 * fork() creates a new process (the "child") that is a near-exact COPY
 * of the calling process (the "parent") - same code, same variable
 * values at the instant of the call - but from that point on the two
 * have entirely SEPARATE address spaces: changes either one makes to
 * its own variables are invisible to the other.
 *
 * fork() returns twice: once in the parent, where it returns the
 * child's new PID, and once in the child, where it returns 0. That is
 * the only way the same code tells the two processes apart.
 *
 * This program demonstrates:
 *   - parent and child having their own PID (getpid()) and both
 *     agreeing on who the parent is (getppid())
 *   - the child modifying a local variable and the parent's copy
 *     being completely unaffected (separate address spaces)
 *   - the child calling exit(n) with a specific status code
 *   - the parent calling wait() to block until the child finishes,
 *     then decoding that status with WIFEXITED()/WEXITSTATUS()
 *
 * Compile: gcc sysio_fork.c -o sysio_fork
 * Run    : ./sysio_fork
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);  /* line-buffer stdout so parent/child
                                           output interleaves in real order
                                           instead of duplicating unflushed
                                           buffered text across fork() */
    int shared_looking_var = 100;

    printf("[Before fork] pid=%d, shared_looking_var=%d\n", getpid(), shared_looking_var);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        /* ---------- CHILD ---------- */
        shared_looking_var = 999;             /* only changes the CHILD's copy */
        printf("[Child]  pid=%d, parent pid=%d, shared_looking_var=%d\n",
               getpid(), getppid(), shared_looking_var);
        printf("[Child]  exiting with status 42\n");
        exit(42);                             /* terminate the child, reporting status 42 */

    } else {
        /* ---------- PARENT ---------- */
        printf("[Parent] pid=%d, created child pid=%d\n", getpid(), pid);

        int status;
        pid_t finished = wait(&status);       /* block until the child terminates */

        printf("[Parent] wait() returned for pid=%d\n", finished);
        if (WIFEXITED(status)) {
            printf("[Parent] child exited normally with status %d\n", WEXITSTATUS(status));
        } else {
            printf("[Parent] child did not exit normally\n");
        }

        /* Proof the two address spaces were separate: the parent's copy
           of shared_looking_var was never touched by the child. */
        printf("[Parent] shared_looking_var is still %d (unaffected by the child)\n",
               shared_looking_var);
    }

    return 0;
}

/*
 * Sample output:
 *
 * [Before fork] pid=5001, shared_looking_var=100
 * [Parent] pid=5001, created child pid=5002
 * [Child]  pid=5002, parent pid=5001, shared_looking_var=999
 * [Child]  exiting with status 42
 * [Parent] wait() returned for pid=5002
 * [Parent] child exited normally with status 42
 * [Parent] shared_looking_var is still 100 (unaffected by the child)
 */
