/*
 * FCFS (First-Come, First-Served) CPU Scheduling - WITH ARRIVAL TIME + GANTT CHART
 * ---------------------------------------------------------------------------------
 * Same scheduling logic as fcfs_arrival.c, but additionally prints a text
 * Gantt chart of the full execution timeline (including any IDLE gaps
 * where the CPU waits for the next process to arrive).
 *
 * Compile: gcc fcfs_gantt.c -o fcfs_gantt
 * Run    : ./fcfs_gantt
 */

#include <stdio.h>

typedef struct {
    int id;
    int arrival;
    int burst;
    int start;
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
        printf("Enter arrival time and burst time for P%d: ", i + 1);
        scanf("%d %d", &p[i].arrival, &p[i].burst);
    }

    /* Stable sort by arrival time */
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++)
            if (p[j].arrival < p[min_idx].arrival) min_idx = j;
        if (min_idx != i) {
            Process temp = p[min_idx];
            for (int k = min_idx; k > i; k--) p[k] = p[k - 1];
            p[i] = temp;
        }
    }

    int current_time = 0;
    float total_wt = 0, total_tat = 0, idle_time = 0;

    /* Gantt chart bookkeeping: one entry per CPU burst/idle segment */
    int g_id[2 * n + 1], g_end[2 * n + 1], g_count = 0;

    for (int i = 0; i < n; i++) {
        if (current_time < p[i].arrival) {
            g_id[g_count] = 0;                 /* 0 marks an IDLE segment */
            g_end[g_count] = p[i].arrival;
            idle_time += (p[i].arrival - current_time);
            g_count++;
            current_time = p[i].arrival;
        }

        p[i].start = current_time;
        p[i].completion = current_time + p[i].burst;

        g_id[g_count] = p[i].id;
        g_end[g_count] = p[i].completion;
        g_count++;

        current_time = p[i].completion;

        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting    = p[i].turnaround - p[i].burst;

        total_wt  += p[i].waiting;
        total_tat += p[i].turnaround;
    }

    printf("\nProcess\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t\t%d\t\t%d\n",
               p[i].id, p[i].arrival, p[i].burst,
               p[i].completion, p[i].turnaround, p[i].waiting);
    }

    printf("\nAverage Waiting Time    = %.2f", total_wt / n);
    printf("\nAverage Turnaround Time = %.2f\n", total_tat / n);

    printf("\nGantt Chart:\n");
    printf("| ");
    for (int i = 0; i < g_count; i++) {
        if (g_id[i] == 0) printf("IDLE | ");
        else printf("P%d | ", g_id[i]);
    }
    printf("\n0");
    for (int i = 0; i < g_count; i++) {
        printf("%*d", 6, g_end[i]);
    }
    printf("\n");

    if (idle_time > 0)
        printf("\nTotal CPU idle time = %.0f units\n", idle_time);

    return 0;
}
