/*
 * FCFS (First-Come, First-Served) CPU Scheduling - WITH ARRIVAL TIME
 * ---------------------------------------------------------------------
 * Processes may arrive at different times. They are scheduled strictly
 * in order of arrival time (ties broken by the order they were entered).
 * The CPU sits idle if the next process in line hasn't arrived yet.
 *
 * Compile: gcc fcfs_arrival.c -o fcfs_arrival
 * Run    : ./fcfs_arrival
 */

#include <stdio.h>

typedef struct {
    int id;
    int arrival;
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
        printf("Enter arrival time and burst time for P%d: ", i + 1);
        scanf("%d %d", &p[i].arrival, &p[i].burst);
    }

    /* Sort by arrival time; ties keep original (input) order -> stable
       selection sort so earlier-entered processes stay earlier on ties */
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (p[j].arrival < p[min_idx].arrival) min_idx = j;
        }
        if (min_idx != i) {
            Process temp = p[min_idx];
            /* shift the block between i and min_idx to preserve stability */
            for (int k = min_idx; k > i; k--) p[k] = p[k - 1];
            p[i] = temp;
        }
    }

    int current_time = 0;
    float total_wt = 0, total_tat = 0;

    for (int i = 0; i < n; i++) {
        if (current_time < p[i].arrival)
            current_time = p[i].arrival;   /* CPU idles until process arrives */

        p[i].completion = current_time + p[i].burst;
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

    return 0;
}
