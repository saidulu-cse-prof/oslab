/*
 * UNIX/LINUX SYSTEM CALLS: fork() + exec() (and exit())
 * ---------------------------------------------------------------------------------
 * fork() alone just gives you two copies of the SAME program running.
 * To run a DIFFERENT program, the usual pattern - used by every shell -
 * is fork() followed immediately by one of the exec() family in the
 * child: exec() replaces the calling process's code, data, and stack
 * with those of a new program, in place, keeping the same PID. It does
 * not return on success (there is no "old" program left to return to);
 * it only returns if it FAILS, in which case it returns -1 and sets
 * errno, and the child should report that and exit().
 *
 * This program:
 *   1. fork()s.
 *   2. In the child, calls execlp("ls", "ls", "-l", filename, NULL) -
 *      execlp() searches $PATH for "ls" and replaces the child process
 *      image with it, passing it the given argv. If execlp() returns
 *      at all, it failed, so the child reports the error and exit()s.
 *   3. In the parent, wait()s for the child and reports its exit
 *      status - demonstrating that the child still had the SAME PID
 *      the whole time, even though it became a completely different
 *      program (ls) partway through its life.
 *   4. For contrast, also demonstrates execlp() FAILING (requesting a
 *      program that doesn't exist), to show that branch of the code.
 *
 * Compile: gcc sysio_fork_exec.c -o sysio_fork_exec
 * Run    : ./sysio_fork_exec
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void runChildAndWait(const char *label, void (*childAction)(void)) {
    printf("\n--- %s ---\n", label);
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        childAction();
        /* If we ever reach this line, exec() failed inside childAction(). */
        _exit(127);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            printf("[Parent] child pid=%d exited with status %d\n", pid, WEXITSTATUS(status));
        else
            printf("[Parent] child pid=%d did not exit normally\n", pid);
    }
}

static void doSuccessfulExec(void) {
    printf("[Child]  pid=%d, about to exec \"ls -l sysio_fork_exec.c\"\n", getpid());
    execlp("ls", "ls", "-l", "sysio_fork_exec.c", NULL);

    /* Only reached if execlp() failed */
    perror("execlp failed");
    exit(1);
}

static void doFailingExec(void) {
    printf("[Child]  pid=%d, about to exec a program that does not exist\n", getpid());
    execlp("this_program_does_not_exist_12345", "this_program_does_not_exist_12345", NULL);

    /* This branch IS reached, because the exec above is expected to fail */
    fprintf(stderr, "[Child]  execlp failed as expected: ");
    perror("");
    exit(1);
}

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);  /* line-buffer stdout: without this,
                                           each fork() would duplicate any
                                           still-unflushed buffered text into
                                           the child, which then prints it a
                                           second time whenever it flushes */
    runChildAndWait("Demo 1: fork() + successful execlp(\"ls\", ...)", doSuccessfulExec);
    runChildAndWait("Demo 2: fork() + execlp() on a nonexistent program (failure path)", doFailingExec);
    return 0;
}

/*
 * Sample output:
 *
 * --- Demo 1: fork() + successful execlp("ls", ...) ---
 * [Child]  pid=7001, about to exec "ls -l sysio_fork_exec.c"
 * -rw-r--r-- 1 user user 2423 Oct 14 09:12 sysio_fork_exec.c
 * [Parent] child pid=7001 exited with status 0
 *
 * --- Demo 2: fork() + execlp() on a nonexistent program (failure path) ---
 * [Child]  pid=7002, about to exec a program that does not exist
 * [Child]  execlp failed as expected: : No such file or directory
 * [Parent] child pid=7002 exited with status 1
 */
