/*
 * Round Robin CPU Scheduling - WITH ARRIVAL TIME
 * ---------------------------------------------------------------------
 * Processes arrive at different times and join the ready queue when
 * their arrival time is reached. Convention used when a process's
 * quantum expires: any process(es) that arrived during that quantum
 * are added to the ready queue FIRST, and only then is the preempted
 * process placed at the back of the queue.
 *
 * Compile: gcc rr_arrival.c -o rr_arrival
 * Run    : ./rr_arrival
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

    /* order[] holds process indices sorted by arrival time (stable) -
       used to bring newly-arrived processes into the ready queue */
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
    int arr_ptr = 0;   /* how many processes (in 'order') have joined the queue so far */

    /* Ready queue - generous size, since a process can be re-queued many times */
    int capacity = 100000;
    int queue[capacity], front = 0, rear = 0;

    int current_time = 0, finished = 0;

    /* bring in every process that has already arrived at time 0 */
    while (arr_ptr < n && arrival[order[arr_ptr]] <= current_time) {
        queue[rear++] = order[arr_ptr++];
    }

    while (finished < n) {
        if (front == rear) {
            /* queue empty but work remains - jump clock to next arrival */
            current_time = arrival[order[arr_ptr]];
            while (arr_ptr < n && arrival[order[arr_ptr]] <= current_time)
                queue[rear++] = order[arr_ptr++];
        }

        int i = queue[front++];
        int slice = (remaining[i] < quantum) ? remaining[i] : quantum;
        current_time += slice;
        remaining[i] -= slice;

        /* enqueue anyone who arrived during this slice, in arrival order */
        while (arr_ptr < n && arrival[order[arr_ptr]] <= current_time)
            queue[rear++] = order[arr_ptr++];

        if (remaining[i] == 0) {
            completion[i] = current_time;
            finished++;
        } else {
            queue[rear++] = i;   /* preempted process re-joins AFTER new arrivals */
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

    return 0;
}
