/*
 * Round Robin CPU Scheduling - BASIC version (no arrival time)
 * -----------------------------------------------------------------
 * Assumes every process is available at time 0. Each process gets the
 * CPU for at most one time quantum per turn, cycling through a simple
 * circular queue until all processes finish. This is the simplest lab
 * version: no arrival-time input and no Gantt chart.
 *
 * Compile: gcc rr_basic.c -o rr_basic
 * Run    : ./rr_basic
 */

#include <stdio.h>

int main() {
    int n, quantum;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    int burst[n], remaining[n], completion[n], turnaround[n], waiting[n];

    for (int i = 0; i < n; i++) {
        printf("Enter burst time for P%d: ", i + 1);
        scanf("%d", &burst[i]);
        remaining[i] = burst[i];
    }

    printf("Enter time quantum: ");
    scanf("%d", &quantum);

    /* Simple circular queue holding process indices */
    int queue[1000], front = 0, rear = 0, qsize = 0;
    for (int i = 0; i < n; i++) { queue[rear++] = i; qsize++; }

    int current_time = 0, finished = 0;

    while (finished < n) {
        int i = queue[front];
        front = (front + 1) % 1000;
        qsize--;

        if (remaining[i] == 0) continue;   /* safety guard, shouldn't happen */

        int slice = (remaining[i] < quantum) ? remaining[i] : quantum;
        current_time += slice;
        remaining[i] -= slice;

        if (remaining[i] == 0) {
            completion[i] = current_time;
            finished++;
        } else {
            queue[rear] = i;               /* not done - go to the back of the queue */
            rear = (rear + 1) % 1000;
            qsize++;
        }
    }

    float total_wt = 0, total_tat = 0;
    for (int i = 0; i < n; i++) {
        turnaround[i] = completion[i];          /* arrival time is 0 for all */
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
