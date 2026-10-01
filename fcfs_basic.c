/*
 * FCFS (First-Come, First-Served) CPU Scheduling - BASIC version
 * ---------------------------------------------------------------
 * Assumes every process is available at time 0 and is scheduled in
 * the exact order it is entered (process order == arrival order).
 * This is the simplest lab version of FCFS: no arrival-time input,
 * no Gantt chart - just completion / turnaround / waiting times.
 *
 * Compile: gcc fcfs_basic.c -o fcfs_basic
 * Run    : ./fcfs_basic
 */

#include <stdio.h>

int main() {
    int n;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    int burst[n], completion[n], turnaround[n], waiting[n];

    for (int i = 0; i < n; i++) {
        printf("Enter burst time for P%d: ", i + 1);
        scanf("%d", &burst[i]);
    }

    /* First process starts at time 0; every later process starts the
       instant the previous one finishes (no gaps, since all arrive at 0) */
    completion[0] = burst[0];
    for (int i = 1; i < n; i++) {
        completion[i] = completion[i - 1] + burst[i];
    }

    float total_wt = 0, total_tat = 0;
    for (int i = 0; i < n; i++) {
        turnaround[i] = completion[i];      /* arrival time is 0 for all */
        waiting[i]    = turnaround[i] - burst[i];
        total_wt  += waiting[i];
        total_tat += turnaround[i];
    }

    printf("\nProcess\tBurst\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t\t%d\t\t%d\n",
               i + 1, burst[i], completion[i], turnaround[i], waiting[i]);
    }

    printf("\nAverage Waiting Time    = %.2f", total_wt / n);
    printf("\nAverage Turnaround Time = %.2f\n", total_tat / n);

    return 0;
}
