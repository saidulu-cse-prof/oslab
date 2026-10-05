/*
 * Round Robin CPU Scheduling - ARRIVAL TIME + FULL GANTT CHART
 * ------------------------------------------------------------------------
 * Same arrival-aware Round Robin logic as rr_arrival.c (new arrivals join
 * the ready queue before a preempted process is re-queued), but this
 * version also records and prints the complete Gantt chart of every CPU
 * time slice, including any IDLE gaps.
 *
 * Compile: gcc rr_gantt.c -o rr_gantt
 * Run    : ./rr_gantt
 */

#include <stdio.h>

int main() {
    int n, quantum;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    int arrival[n], burst[n], remaining[n], completion[n], turnaround[n], waiting[n];

    for (int i = 0; i < n; i++) {
        printf("Enter arrival time and burst time for P%d: ", i + 1);
        scanf("%d %d", &arrival[i], &burst[i]);
        remaining[i] = burst[i];
    }

    printf("Enter time quantum: ");
    scanf("%d", &quantum);

    int order[n];
    for (int i = 0; i < n; i++) order[i] = i;
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++)
            if (arrival[order[j]] < arrival[order[min_idx]]) min_idx = j;
        if (min_idx != i) {
            int temp = order[min_idx];
            for (int k = min_idx; k > i; k--) order[k] = order[k - 1];
            order[i] = temp;
        }
    }
    int arr_ptr = 0;

    int capacity = 100000;
    int queue[capacity], front = 0, rear = 0;

    /* Gantt chart log: one entry per CPU slice (0 = IDLE) */
    int g_id[20000], g_start[20000], g_end[20000], g_count = 0;

    int current_time = 0, finished = 0;
    float idle_time = 0;

    while (arr_ptr < n && arrival[order[arr_ptr]] <= current_time)
        queue[rear++] = order[arr_ptr++];

    while (finished < n) {
        if (front == rear) {
            int next_arrival = arrival[order[arr_ptr]];
            if (next_arrival > current_time) {
                g_id[g_count] = -1;              /* -1 marks an IDLE segment */
                g_start[g_count] = current_time;
                g_end[g_count] = next_arrival;
                idle_time += (next_arrival - current_time);
                g_count++;
            }
            current_time = next_arrival;
            while (arr_ptr < n && arrival[order[arr_ptr]] <= current_time)
                queue[rear++] = order[arr_ptr++];
        }

        int i = queue[front++];
        int start = current_time;
        int slice = (remaining[i] < quantum) ? remaining[i] : quantum;
        current_time += slice;
        remaining[i] -= slice;

        g_id[g_count] = i + 1;
        g_start[g_count] = start;
        g_end[g_count] = current_time;
        g_count++;

        while (arr_ptr < n && arrival[order[arr_ptr]] <= current_time)
            queue[rear++] = order[arr_ptr++];

        if (remaining[i] == 0) {
            completion[i] = current_time;
            finished++;
        } else {
            queue[rear++] = i;
        }
    }

    float total_wt = 0, total_tat = 0;
    for (int i = 0; i < n; i++) {
        turnaround[i] = completion[i] - arrival[i];
        waiting[i]    = turnaround[i] - burst[i];
        total_wt  += waiting[i];
        total_tat += turnaround[i];
    }

    printf("\nProcess\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t\t%d\t\t%d\n",
               i + 1, arrival[i], burst[i],
               completion[i], turnaround[i], waiting[i]);
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

    if (idle_time > 0)
        printf("\nTotal CPU idle time = %.0f units\n", idle_time);

    return 0;
}
