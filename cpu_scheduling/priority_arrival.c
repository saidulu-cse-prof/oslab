/*
 * Priority CPU Scheduling - NON-PREEMPTIVE, WITH ARRIVAL TIME
 * -------------------------------------------------------------------------------
 * At every decision point, the scheduler looks at all processes that have
 * ARRIVED but not yet run, and picks the one with the highest priority
 * (LOWER number = HIGHER priority). Ties are broken by earlier arrival,
 * then by input order. Once a process starts running it is NOT preempted,
 * even if a higher-priority process arrives later (see
 * priority_preemptive_gantt.c for the preemptive version).
 *
 * Compile: gcc priority_arrival.c -o priority_arrival
 * Run    : ./priority_arrival
 */

#include <stdio.h>
#include <limits.h>

typedef struct {
    int id;
    int arrival;
    int burst;
    int priority;
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
        printf("Enter arrival time, burst time and priority for P%d (lower number = higher priority): ",
               i + 1);
        scanf("%d %d %d", &p[i].arrival, &p[i].burst, &p[i].priority);
    }

    int completed = 0, current_time = 0;
    float total_wt = 0, total_tat = 0;

    while (completed < n) {
        /* Among processes that have arrived and are not yet done,
           find the one with the best (lowest-numbered) priority. */
        int chosen = -1, best_priority = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!p[i].done && p[i].arrival <= current_time) {
                if (p[i].priority < best_priority ||
                   (p[i].priority == best_priority && p[i].arrival < p[chosen].arrival)) {
                    best_priority = p[i].priority;
                    chosen = i;
                }
            }
        }

        if (chosen == -1) {
            /* Nobody has arrived yet - jump the clock to the next arrival */
            int next_arrival = INT_MAX;
            for (int i = 0; i < n; i++)
                if (!p[i].done && p[i].arrival < next_arrival)
                    next_arrival = p[i].arrival;
            current_time = next_arrival;
            continue;
        }

        p[chosen].completion = current_time + p[chosen].burst;
        current_time = p[chosen].completion;

        p[chosen].turnaround = p[chosen].completion - p[chosen].arrival;
        p[chosen].waiting    = p[chosen].turnaround - p[chosen].burst;
        p[chosen].done = 1;
        completed++;

        total_wt  += p[chosen].waiting;
        total_tat += p[chosen].turnaround;
    }

    printf("\nProcess\tArrival\tBurst\tPriority\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t\t%d\t\t%d\t\t%d\n",
               p[i].id, p[i].arrival, p[i].burst, p[i].priority,
               p[i].completion, p[i].turnaround, p[i].waiting);
    }

    printf("\nAverage Waiting Time    = %.2f", total_wt / n);
    printf("\nAverage Turnaround Time = %.2f\n", total_tat / n);

    return 0;
}
