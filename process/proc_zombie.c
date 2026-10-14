/*
 * PROCESS CONCEPT: ZOMBIE PROCESSES
 * ---------------------------------------------------------------------------------
 * When a child process terminates, the kernel does NOT let it disappear
 * immediately - it keeps a small record (PID, exit status, resource
 * usage) around until the parent calls wait()/waitpid() to "reap" it.
 * Until that happens, the terminated child is a ZOMBIE (shown by ps as
 * state 'Z' and command name in parentheses, e.g. "(proc_zombie)").
 * A zombie holds almost no resources (its memory and file descriptors
 * are already freed) but it DOES occupy a process-table slot/PID, which
 * is why large numbers of un-reaped zombies are a real (if usually
 * slow-building) resource leak.
 *
 * This program creates exactly that situation on purpose:
 *   1. fork() a child that exits almost immediately.
 *   2. The PARENT deliberately does NOT call wait() right away - it
 *      sleeps instead, giving the child time to finish and become a
 *      zombie.
 *   3. While the parent sleeps, we shell out to `ps` to show the
 *      child's entry with STAT = Z (zombie / "defunct").
 *   4. The parent then calls wait() to reap it, and we run the same
 *      `ps` check again to show the zombie is now gone.
 *
 * Compile: gcc proc_zombie.c -o proc_zombie
 * Run    : ./proc_zombie
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void showProcessStatus(pid_t pid) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "ps -o pid,ppid,stat,cmd -p %d 2>/dev/null", pid);
    printf("  $ %s\n", cmd);
    FILE *p = popen(cmd, "r");
    if (!p) { perror("popen failed"); return; }

    /* `ps -p <pid>` always prints its header row even when nothing
       matches, so buffer the output and only show it if there is an
       actual DATA row (line 2 onward) - otherwise report plainly that
       the process is gone, instead of printing a header with nothing
       under it. */
    char lines[4][256];
    int lineCount = 0;
    while (lineCount < 4 && fgets(lines[lineCount], sizeof(lines[lineCount]), p))
        lineCount++;
    pclose(p);

    if (lineCount <= 1) {
        printf("  (no such process - already reaped)\n");
    } else {
        for (int i = 0; i < lineCount; i++) printf("  %s", lines[i]);
    }
}

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    printf("[Parent] pid=%d, forking a short-lived child...\n", getpid());
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        /* ---------- CHILD: exits almost immediately ---------- */
        printf("[Child]  pid=%d, exiting right away\n", getpid());
        exit(0);
    }

    /* ---------- PARENT ---------- */
    printf("[Parent] sleeping for 2 seconds WITHOUT calling wait() yet,\n");
    printf("[Parent] so child pid=%d has time to exit and become a zombie...\n\n", pid);
    sleep(2);

    printf("While the child is unreaped, here is its process-table entry:\n");
    showProcessStatus(pid);

    printf("\n[Parent] now calling wait() to reap it...\n");
    int status;
    wait(&status);
    printf("[Parent] reaped pid=%d, exit status=%d\n\n", pid, WEXITSTATUS(status));

    printf("After reaping, the same ps lookup finds nothing:\n");
    showProcessStatus(pid);

    return 0;
}

/*
 * Sample output:
 *
 * [Parent] pid=9001, forking a short-lived child...
 * [Child]  pid=9002, exiting right away
 * [Parent] sleeping for 2 seconds WITHOUT calling wait() yet,
 * [Parent] so child pid=9002 has time to exit and become a zombie...
 *
 * While the child is unreaped, here is its process-table entry:
 *   $ ps -o pid,ppid,stat,cmd -p 9002 2>/dev/null
 *    9002  9001 Z    [proc_zombie] <defunct>
 *
 * [Parent] now calling wait() to reap it...
 * [Parent] reaped pid=9002, exit status=0
 *
 * After reaping, the same ps lookup finds nothing:
 *   $ ps -o pid,ppid,stat,cmd -p 9002 2>/dev/null
 *   (no such process - already reaped)
 *
 * (ps actually still prints its header row even when no process
 * matches - this program accounts for that and reports plainly that
 * the process is gone rather than showing a header with nothing
 * under it.)
 */
