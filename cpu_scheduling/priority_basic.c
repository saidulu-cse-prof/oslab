/*
 * Priority CPU Scheduling - BASIC version (non-preemptive, no arrival time)
 * ------------------------------------------------------------------------------
 * Assumes every process is available at time 0. Processes are simply
 * sorted by priority (LOWER number = HIGHER priority - the usual
 * convention) and run to completion in that order. This is the
 * simplest lab version: no arrival-time input and no Gantt chart.
 *
 * Compile: gcc priority_basic.c -o priority_basic
 * Run    : ./priority_basic
 */

#include <stdio.h>

typedef struct {
    int id;
    int burst;
    int priority;
    int completion;
    int turnaround;
    int waiting;
} Process;

int main() {
    int n;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    Process p[n];
    for (int i = 0; i < n; i++) {
        p[i].id = i + 1;
        printf("Enter burst time and priority for P%d (lower number = higher priority): ",
               i + 1);
        scanf("%d %d", &p[i].burst, &p[i].priority);
    }

    /* Stable sort by priority (ascending -> highest priority first) */
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++)
            if (p[j].priority < p[min_idx].priority) min_idx = j;
        if (min_idx != i) {
            Process temp = p[min_idx];
            for (int k = min_idx; k > i; k--) p[k] = p[k - 1];
            p[i] = temp;
        }
    }

    float total_wt = 0, total_tat = 0;
    p[0].completion = p[0].burst;
    for (int i = 1; i < n; i++) {
        p[i].completion = p[i - 1].completion + p[i].burst;
    }

    for (int i = 0; i < n; i++) {
        p[i].turnaround = p[i].completion;      /* arrival time is 0 for all */
        p[i].waiting    = p[i].turnaround - p[i].burst;
        total_wt  += p[i].waiting;
        total_tat += p[i].turnaround;
    }

    printf("\n(Processes below are listed in the order they run, highest priority first)\n");
    printf("Process\tPriority\tBurst\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t\t%d\t%d\t\t%d\t\t%d\n",
               p[i].id, p[i].priority, p[i].burst,
               p[i].completion, p[i].turnaround, p[i].waiting);
    }

    printf("\nAverage Waiting Time    = %.2f", total_wt / n);
    printf("\nAverage Turnaround Time = %.2f\n", total_tat / n);

    return 0;
}
