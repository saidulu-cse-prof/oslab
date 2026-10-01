/*
 * SJF (Shortest Job First) CPU Scheduling - BASIC version (non-preemptive)
 * --------------------------------------------------------------------------
 * Assumes every process is available at time 0. Processes are simply
 * sorted by burst time (shortest first) and run to completion in that
 * order - the simplest lab version of SJF, with no arrival-time input
 * and no Gantt chart.
 *
 * Compile: gcc sjf_basic.c -o sjf_basic
 * Run    : ./sjf_basic
 */

#include <stdio.h>

typedef struct {
    int id;
    int burst;
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
        printf("Enter burst time for P%d: ", i + 1);
        scanf("%d", &p[i].burst);
    }

    /* Stable sort by burst time (shortest job first) */
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++)
            if (p[j].burst < p[min_idx].burst) min_idx = j;
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

    printf("\n(Processes below are listed in the order they run, shortest burst first)\n");
    printf("Process\tBurst\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t\t%d\t\t%d\n",
               p[i].id, p[i].burst, p[i].completion, p[i].turnaround, p[i].waiting);
    }

    printf("\nAverage Waiting Time    = %.2f", total_wt / n);
    printf("\nAverage Turnaround Time = %.2f\n", total_tat / n);

    return 0;
}
