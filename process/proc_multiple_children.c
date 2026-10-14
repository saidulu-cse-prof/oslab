/*
 * PROCESS CONCEPT: CREATING AND REAPING MULTIPLE CHILD PROCESSES
 * ---------------------------------------------------------------------------------
 * A parent is not limited to one child - calling fork() N times in a
 * loop creates N independent children, each a snapshot of the parent
 * at the moment IT called fork(). This program creates several
 * children that each take a DIFFERENT amount of time to finish, then
 * has the parent collect them with waitpid(-1, &status, 0) in a loop.
 *
 * The key thing to notice: waitpid(-1, ...) waits for WHICHEVER child
 * finishes next - so the parent reaps them in COMPLETION order, which
 * is generally NOT the same as the order they were created in (the
 * child that sleeps the least finishes first, regardless of when it
 * was fork()'d). Each child also exits with a distinct status code so
 * the parent can report exactly which child it just reaped and what
 * that child's result was.
 *
 * Compile: gcc proc_multiple_children.c -o proc_multiple_children
 * Run    : ./proc_multiple_children
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_CHILDREN 4

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    /* Each child sleeps a different amount so they finish out of order. */
    int sleep_seconds[NUM_CHILDREN] = {3, 1, 2, 0};
    pid_t child_pids[NUM_CHILDREN];

    printf("[Parent] pid=%d, creating %d children...\n", getpid(), NUM_CHILDREN);

    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {
            /* ---------- CHILD i ---------- */
            printf("[Child %d] pid=%d, will sleep %ds then exit with status %d\n",
                   i, getpid(), sleep_seconds[i], (i + 1) * 10);
            sleep(sleep_seconds[i]);
            exit((i + 1) * 10);          /* distinct exit status per child */
        } else {
            /* ---------- PARENT: just record the pid and keep forking ---------- */
            child_pids[i] = pid;
        }
    }

    printf("[Parent] all %d children created: ", NUM_CHILDREN);
    for (int i = 0; i < NUM_CHILDREN; i++) printf("%d ", child_pids[i]);
    printf("\n[Parent] now reaping them in COMPLETION order with waitpid(-1, ...):\n");

    for (int reaped = 1; reaped <= NUM_CHILDREN; reaped++) {
        int status;
        pid_t finished = waitpid(-1, &status, 0);   /* -1 = "any child" */

        /* Figure out which logical child index this was, just for the report. */
        int index = -1;
        for (int i = 0; i < NUM_CHILDREN; i++)
            if (child_pids[i] == finished) { index = i; break; }

        if (WIFEXITED(status)) {
            printf("  [%d/%d] reaped pid=%d (child #%d) - exit status %d\n",
                   reaped, NUM_CHILDREN, finished, index, WEXITSTATUS(status));
        } else {
            printf("  [%d/%d] reaped pid=%d (child #%d) - did not exit normally\n",
                   reaped, NUM_CHILDREN, finished, index);
        }
    }

    printf("[Parent] all children reaped. Done.\n");
    return 0;
}

/*
 * Sample output (sleep times are 3,1,2,0 for children 0,1,2,3 - so the
 * expected COMPLETION order is child 3, then 1, then 2, then 0; the
 * four "[Child N] ... will sleep ..." STARTUP lines, on the other hand,
 * are printed by each child independently right after its own fork()
 * returns, so their relative order can vary slightly run to run - that
 * is ordinary scheduling non-determinism, not a bug):
 *
 * [Parent] pid=9201, creating 4 children...
 * [Child 0] pid=9202, will sleep 3s then exit with status 10
 * [Child 1] pid=9203, will sleep 1s then exit with status 20
 * [Child 2] pid=9204, will sleep 2s then exit with status 30
 * [Child 3] pid=9205, will sleep 0s then exit with status 40
 * [Parent] all 4 children created: 9202 9203 9204 9205
 * [Parent] now reaping them in COMPLETION order with waitpid(-1, ...):
 *   [1/4] reaped pid=9205 (child #3) - exit status 40
 *   [2/4] reaped pid=9203 (child #1) - exit status 20
 *   [3/4] reaped pid=9204 (child #2) - exit status 30
 *   [4/4] reaped pid=9202 (child #0) - exit status 10
 * [Parent] all children reaped. Done.
 */
