/*
 * Priority CPU Scheduling - PREEMPTIVE, WITH ARRIVAL TIME + GANTT CHART
 * --------------------------------------------------------------------------------
 * LOWER number = HIGHER priority. Whenever a new process arrives with a
 * better (lower-numbered) priority than the one currently running, the
 * CPU is preempted and switched to the new arrival. Ties in priority are
 * broken in favor of whichever process is already running (to avoid an
 * unnecessary context switch), then by earlier arrival, then by input order.
 *
 * Simulated one time unit at a time, which keeps the preemption logic easy
 * to follow and is more than fast enough for typical lab-sized inputs.
 *
 * Compile: gcc priority_preemptive_gantt.c -o priority_preemptive_gantt
 * Run    : ./priority_preemptive_gantt
 */

#include <stdio.h>
#include <limits.h>

typedef struct {
    int id;
    int arrival;
    int burst;
    int priority;
    int remaining;
    int completion;
    int turnaround;
    int waiting;
} Process;

int main() {
    int n;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    Process p[n];
    int max_arrival = 0, total_burst = 0;
    for (int i = 0; i < n; i++) {
        p[i].id = i + 1;
        printf("Enter arrival time, burst time and priority for P%d (lower number = higher priority): ",
               i + 1);
        scanf("%d %d %d", &p[i].arrival, &p[i].burst, &p[i].priority);
        p[i].remaining = p[i].burst;
        if (p[i].arrival > max_arrival) max_arrival = p[i].arrival;
        total_burst += p[i].burst;
    }

    int completed = 0, current_time = 0;
    int running = -1;                  /* index of process running in the previous tick */

    /* Gantt chart log: one entry per contiguous CPU segment (-1 = IDLE) */
    int cap = total_burst + max_arrival + 10;
    int g_id[cap], g_start[cap], g_end[cap], g_count = 0;
    int seg_id = -2;                   /* sentinel: "no segment open yet" */

    while (completed < n) {
        /* Pick the best-priority process that has arrived and still has work.
           On a priority tie, prefer the process already running. */
        int chosen = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= current_time) {
                if (chosen == -1) {
                    chosen = i;
                } else if (p[i].priority < p[chosen].priority) {
                    chosen = i;
                } else if (p[i].priority == p[chosen].priority) {
                    if (i == running) chosen = i;
                    else if (chosen != running &&
                             (p[i].arrival < p[chosen].arrival)) chosen = i;
                }
            }
        }

        int tick_id = (chosen == -1) ? -1 : p[chosen].id;   /* -1 = IDLE this tick */

        if (tick_id != seg_id) {
            if (seg_id != -2) g_end[g_count - 1] = current_time;   /* close previous segment */
            g_id[g_count] = tick_id;
            g_start[g_count] = current_time;
            g_count++;
            seg_id = tick_id;
        }

        if (chosen != -1) {
            p[chosen].remaining--;
            running = chosen;
            if (p[chosen].remaining == 0) {
                p[chosen].completion = current_time + 1;
                p[chosen].turnaround = p[chosen].completion - p[chosen].arrival;
                p[chosen].waiting    = p[chosen].turnaround - p[chosen].burst;
                completed++;
                running = -1;
            }
        } else {
            running = -1;
        }

        current_time++;
    }
    g_end[g_count - 1] = current_time;   /* close the final segment */

    float total_wt = 0, total_tat = 0;
    for (int i = 0; i < n; i++) {
        total_wt  += p[i].waiting;
        total_tat += p[i].turnaround;
    }

    printf("\nProcess\tArrival\tBurst\tPriority\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t\t%d\t\t%d\t\t%d\n",
               p[i].id, p[i].arrival, p[i].burst, p[i].priority,
               p[i].completion, p[i].turnaround, p[i].waiting);
    }

    printf("\nAverage Waiting Time    = %.2f", total_wt / n);
    printf("\nAverage Turnaround Time = %.2f\n", total_tat / n);

    printf("\nGantt Chart:\n");
    printf("| ");
    for (int i = 0; i < g_count; i++) {
        if (g_id[i] == -1) printf("IDLE | ");
        else printf("P%d | ", g_id[i]);
    }
    printf("\n%d", g_start[0]);
    for (int i = 0; i < g_count; i++) {
        printf("%*d", 6, g_end[i]);
    }
    printf("\n");

    return 0;
}
