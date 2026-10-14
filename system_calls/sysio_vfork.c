/*
 * UNIX/LINUX SYSTEM CALL: vfork()
 * ---------------------------------------------------------------------------------
 * vfork() creates a new process like fork(), but with two important
 * differences that this program exists to demonstrate:
 *
 *   1. The child does NOT get its own copy of the address space -
 *      it runs IN the parent's address space (traditionally this was
 *      vfork's whole performance advantage: no copy-on-write page
 *      tables to set up). This means a global/variable the child
 *      modifies is visible to the parent too, once the parent resumes
 *      - the exact OPPOSITE of fork(), where the two are isolated.
 *
 *   2. The PARENT is suspended the instant vfork() returns, and stays
 *      suspended until the child calls _exit() or one of the exec()
 *      family. The child MUST do one of those two very soon - per the
 *      vfork() man page, modifying any variable other than the return
 *      value of vfork() itself, or calling any function other than
 *      _exit()/exec() before doing so, is undefined behavior (because
 *      the child is still borrowing the parent's stack). That's why
 *      this program's child uses write() (a raw system call, not
 *      buffered stdio) and then calls _exit() immediately, rather
 *      than printf() + exit().
 *
 * Compile: gcc sysio_vfork.c -o sysio_vfork
 * Run    : ./sysio_vfork
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static int shared_var = 100;      /* global: this is what the child will change */

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);  /* line-buffer stdout so the parent's
                                           own printf output appears in real
                                           chronological order */
    printf("[Before vfork] pid=%d, shared_var=%d\n", getpid(), shared_var);

    pid_t pid = vfork();

    if (pid < 0) {
        perror("vfork failed");
        exit(1);
    } else if (pid == 0) {
        /* ---------- CHILD ----------
           Running inside the PARENT's address space; the parent is
           suspended right now. Use only write() (not printf/exit) and
           call _exit() immediately, per vfork()'s contract. */
        shared_var = 999;

        const char *msg = "[Child]  (inside parent's address space) set shared_var = 999, calling _exit()\n";
        write(STDOUT_FILENO, msg, strlen(msg));

        _exit(42);                 /* MUST be _exit() (or exec), not exit() */

    } else {
        /* ---------- PARENT ----------
           Resumes only after the child has called _exit() (or exec).
           Because the child ran in OUR address space, its change to
           shared_var is visible right here. */
        printf("[Parent] resumed after child pid=%d finished\n", pid);
        printf("[Parent] shared_var is now %d - changed by the child, unlike fork()!\n",
               shared_var);

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            printf("[Parent] child's exit status was %d\n", WEXITSTATUS(status));
    }

    return 0;
}

/*
 * Sample output:
 *
 * [Before vfork] pid=6001, shared_var=100
 * [Child]  (inside parent's address space) set shared_var = 999, calling _exit()
 * [Parent] resumed after child pid=6002 finished
 * [Parent] shared_var is now 999 - changed by the child, unlike fork()!
 * [Parent] child's exit status was 42
 *
 * Contrast this shared_var=999 result with sysio_fork.c, where the
 * exact same kind of modification by the child left the parent's copy
 * of the variable completely untouched.
 */
