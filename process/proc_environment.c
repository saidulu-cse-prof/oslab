/*
 * PROCESS CONCEPT: THE PROCESS ENVIRONMENT
 * ---------------------------------------------------------------------------------
 * Every process has its own "environment" - a list of NAME=value
 * strings (accessible in C as the global `char **environ`, and looked
 * up one at a time with getenv()). This program demonstrates three
 * related facts about it:
 *
 *   1. A process can add to its OWN environment with setenv().
 *   2. fork() gives the CHILD a full copy of the parent's environment
 *      at that moment - so the child sees whatever the parent had set,
 *      with getenv() working identically in both.
 *   3. exec() is where it gets interesting: execve() lets the caller
 *      hand the new program a COMPLETELY DIFFERENT envp array instead
 *      of inheriting the current environment unchanged. To prove it,
 *      the child here execve()s /usr/bin/env (which just prints
 *      whatever environment IT was given) with a custom envp that does
 *      NOT include the variable the parent set - showing the new
 *      program only sees what was explicitly handed to it.
 *      (Contrast this with execlp()/execvp(), used in the fork+exec
 *      demo elsewhere, which implicitly reuse the CALLING process's
 *      current environ - execve() is the one that gives full control.)
 *
 * Compile: gcc proc_environment.c -o proc_environment
 * Run    : ./proc_environment
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

extern char **environ;     /* the process's own environment list */

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    /* ---- 1. Parent sets a variable in its OWN environment ---- */
    setenv("MY_CUSTOM_VAR", "hello_from_parent", 1);
    printf("[Parent] pid=%d, set MY_CUSTOM_VAR=%s\n", getpid(), getenv("MY_CUSTOM_VAR"));

    int env_count = 0;
    for (char **e = environ; *e != NULL; e++) env_count++;
    printf("[Parent] my environment currently has %d variables\n\n", env_count);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        /* ---------- CHILD ---------- */

        /* ---- 2. fork() copied the environment - the child sees it too ---- */
        printf("[Child]  pid=%d, I inherited MY_CUSTOM_VAR=%s via fork()\n",
               getpid(), getenv("MY_CUSTOM_VAR"));

        /* ---- 3. Now exec /usr/bin/env with a DELIBERATELY DIFFERENT
                   environment that does not include MY_CUSTOM_VAR at all. ---- */
        char *custom_envp[] = {
            (char *) "DIFFERENT_VAR=hello_from_execve",
            (char *) "PATH=/usr/bin:/bin",
            NULL
        };
        printf("[Child]  about to execve(\"/usr/bin/env\") with a brand-new envp\n");
        printf("[Child]  (containing only DIFFERENT_VAR and PATH - NOT MY_CUSTOM_VAR)\n");
        printf("[Child]  ---- output of /usr/bin/env below ----\n");

        execve("/usr/bin/env", (char *[]){ (char *) "/usr/bin/env", NULL }, custom_envp);

        /* Only reached if execve() failed */
        perror("execve failed");
        exit(1);

    } else {
        /* ---------- PARENT ---------- */
        int status;
        waitpid(pid, &status, 0);
        printf("\n[Parent] child pid=%d finished with status %d\n", pid, WEXITSTATUS(status));
        printf("[Parent] my own MY_CUSTOM_VAR is still: %s (execve only changed the CHILD)\n",
               getenv("MY_CUSTOM_VAR"));
    }

    return 0;
}

/*
 * Sample output:
 *
 * [Parent] pid=9301, set MY_CUSTOM_VAR=hello_from_parent
 * [Parent] my environment currently has 23 variables
 *
 * [Child]  pid=9302, I inherited MY_CUSTOM_VAR=hello_from_parent via fork()
 * [Child]  about to execve("/usr/bin/env") with a brand-new envp
 * [Child]  (containing only DIFFERENT_VAR and PATH - NOT MY_CUSTOM_VAR)
 * [Child]  ---- output of /usr/bin/env below ----
 * DIFFERENT_VAR=hello_from_execve
 * PATH=/usr/bin:/bin
 *
 * [Parent] child pid=9302 finished with status 0
 * [Parent] my own MY_CUSTOM_VAR is still: hello_from_parent (execve only changed the CHILD)
 *
 * Note that /usr/bin/env's output shows ONLY DIFFERENT_VAR and PATH -
 * MY_CUSTOM_VAR, which the child itself could still see right up until
 * the execve() call, is gone, because execve() replaced the entire
 * environment rather than adding to it.
 */
