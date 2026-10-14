/*
 * PROCESS CONCEPT: ORPHAN PROCESSES
 * ---------------------------------------------------------------------------------
 * An ORPHAN is a child process whose original parent terminates while
 * the child is still running. The child is not left parentless: the
 * kernel immediately "re-parents" it to a reaper process - traditionally
 * PID 1 (init/systemd), though modern Linux also lets any ancestor mark
 * itself a "subreaper" (PR_SET_CHILD_SUBREAPER) to catch its own
 * descendants' orphans instead; inside a container, the reaper is
 * whatever process is PID 1 *in that container* (often a lightweight
 * init, not necessarily the host's systemd).
 *
 * This program demonstrates the re-parenting directly:
 *   1. fork() a child that SLEEPS for a few seconds (long enough to
 *      outlive its parent) before checking getppid().
 *   2. The PARENT does NOT wait for the child at all - it just prints
 *      a message and exits immediately, abandoning the still-sleeping
 *      child.
 *   3. Once the child wakes up, its getppid() no longer returns the
 *      original parent's PID (which no longer exists) - it returns
 *      whatever reaper process adopted it.
 *
 * Compile: gcc proc_orphan.c -o proc_orphan
 * Run    : ./proc_orphan
 * (Run it plainly, e.g. "./proc_orphan; sleep 3" if you want your shell
 *  to pause long enough to see the child's delayed output before the
 *  prompt returns, since the child keeps running after this program's
 *  own parent process has already exited.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    pid_t original_parent = getpid();
    printf("[Parent] pid=%d, forking a child that will outlive me...\n", original_parent);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        /* ---------- CHILD ---------- */
        printf("[Child]  pid=%d, parent is currently pid=%d. Sleeping 3s...\n",
               getpid(), getppid());
        sleep(3);     /* long enough for the parent below to have exited by now */

        printf("[Child]  pid=%d woke up. My ORIGINAL parent was pid=%d.\n",
               getpid(), original_parent);
        printf("[Child]  getppid() now returns %d - I was re-parented because my\n",
               getppid());
        printf("[Child]  original parent (pid=%d) already exited. I am an ORPHAN.\n",
               original_parent);
        exit(0);
    }

    /* ---------- PARENT ---------- */
    printf("[Parent] created child pid=%d, but exiting immediately WITHOUT waiting.\n", pid);
    printf("[Parent] pid=%d exiting now.\n", getpid());
    /* Deliberately no wait()/waitpid() here - that is what orphans the child. */
    return 0;
}

/*
 * Sample output (the last three "[Child]" lines print about 3 seconds
 * after the first lines, once this program's parent process has
 * already exited):
 *
 * [Parent] pid=9101, forking a child that will outlive me...
 * [Parent] created child pid=9102, but exiting immediately WITHOUT waiting.
 * [Parent] pid=9101 exiting now.
 * [Child]  pid=9102, parent is currently pid=1. Sleeping 3s...
 * [Child]  pid=9102 woke up. My ORIGINAL parent was pid=9101.
 * [Child]  getppid() now returns 1 - I was re-parented because my
 * [Child]  original parent (pid=9101) already exited. I am an ORPHAN.
 *
 * (The exact reaper PID printed depends on your system/container - it
 * is commonly 1, but may be a different "subreaper" PID. Also, because
 * the parent does so little work before exiting, the re-parenting can
 * sometimes complete before the child's very FIRST print statement
 * runs - so that line may already show the new parent instead of the
 * original one. Either way, the child's closing message always shows
 * the re-parenting clearly.)
 */
