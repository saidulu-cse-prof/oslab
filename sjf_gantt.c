/*
 * SJF (Shortest Job First) CPU Scheduling - NON-PREEMPTIVE, ARRIVAL TIME + GANTT CHART
 * ------------------------------------------------------------------------------------------
 * Same non-preemptive SJF logic as sjf_arrival.c, but additionally records
 * and prints a text Gantt chart of the full execution order, including any
 * IDLE gaps where the CPU has no arrived process to run.
 *
 * Compile: gcc sjf_gantt.c -o sjf_gantt
 * Run    : ./sjf_gantt
 */

#include <stdio.h>
#include <limits.h>

typedef struct {
    int id;
    int arrival;
    int burst;
    int completion;
    int turnaround;
    int waiting;
    int done;
} Process;

int main() {
    int n;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    Process p[n];
    for (int i = 0; i < n; i++) {
        p[i].id = i + 1;
        p[i].done = 0;
        printf("Enter arrival time and burst time for P%d: ", i + 1);
        scanf("%d %d", &p[i].arrival, &p[i].burst);
    }

    int completed = 0, current_time = 0;
    float total_wt = 0, total_tat = 0, idle_time = 0;

    int g_id[2 * n + 1], g_end[2 * n + 1], g_count = 0;

    while (completed < n) {
        int chosen = -1, best_burst = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!p[i].done && p[i].arrival <= current_time) {
                if (p[i].burst < best_burst ||
                   (p[i].burst == best_burst && p[i].arrival < p[chosen].arrival)) {
                    best_burst = p[i].burst;
                    chosen = i;
                }
            }
        }

        if (chosen == -1) {
            int next_arrival = INT_MAX;
            for (int i = 0; i < n; i++)
                if (!p[i].done && p[i].arrival < next_arrival)
                    next_arrival = p[i].arrival;

            g_id[g_count] = 0;                 /* 0 marks an IDLE segment */
            g_end[g_count] = next_arrival;
            idle_time += (next_arrival - current_time);
            g_count++;

            current_time = next_arrival;
            continue;
        }

        p[chosen].completion = current_time + p[chosen].burst;
        current_time = p[chosen].completion;

        g_id[g_count] = p[chosen].id;
        g_end[g_count] = p[chosen].completion;
        g_count++;

        p[chosen].turnaround = p[chosen].completion - p[chosen].arrival;
        p[chosen].waiting    = p[chosen].turnaround - p[chosen].burst;
        p[chosen].done = 1;
        completed++;

        total_wt  += p[chosen].waiting;
        total_tat += p[chosen].turnaround;
    }

    printf("\nProcess\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t\t%d\t\t%d\n",
               p[i].id, p[i].arrival, p[i].burst,
               p[i].completion, p[i].turnaround, p[i].waiting);
    }

    printf("\nAverage Waiting Time    = %.2f", total_wt / n);
    printf("\nAverage Turnaround Time = %.2f\n", total_tat / n);

    printf("\nGantt Chart (execution order):\n");
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
