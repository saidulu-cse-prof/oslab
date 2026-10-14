/*
 * DINING PHILOSOPHERS - Resource (Fork) Ordering Solution
 * ---------------------------------------------------------------------
 * N philosophers sit around a circular table with N forks between
 * them (fork i is shared by philosopher i and philosopher (i+1)%N).
 * To eat, a philosopher needs BOTH adjacent forks. The naive "always
 * pick up your left fork, then your right fork" rule can deadlock if
 * every philosopher grabs their left fork at the same instant - each
 * then waits forever for their right fork, which is somebody else's
 * left fork. That is a circular wait, one of the four conditions
 * required for deadlock.
 *
 * This program breaks the circular wait by imposing a strict GLOBAL
 * ORDER on fork acquisition: every philosopher always locks the
 * LOWER-numbered of their two forks first, then the higher-numbered
 * one - never the other way around, regardless of which is "left" or
 * "right" for that philosopher. Since every thread agrees on the same
 * order, no cycle of waiting can ever form, so the system can never
 * deadlock.
 *
 * Compile: gcc dp_resource_ordering.c -o dp_resource_ordering -lpthread
 * Run    : ./dp_resource_ordering
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_PHILOSOPHERS 5
#define MEALS_PER_PHILOSOPHER 3

static pthread_mutex_t forks[NUM_PHILOSOPHERS];
static int meals_eaten[NUM_PHILOSOPHERS];

void *philosopher(void *arg) {
    int id = *(int *) arg;
    int left  = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;

    /* Always acquire the lower-numbered fork first -> a total order on
       all forks, which rules out circular wait by construction. */
    int first  = (left < right) ? left  : right;
    int second = (left < right) ? right : left;

    for (int meal = 1; meal <= MEALS_PER_PHILOSOPHER; meal++) {
        printf("Philosopher %d is THINKING (meal %d/%d)\n", id, meal, MEALS_PER_PHILOSOPHER);
        usleep(5000);

        printf("Philosopher %d is HUNGRY, reaching for fork %d then fork %d\n", id, first, second);
        pthread_mutex_lock(&forks[first]);
        pthread_mutex_lock(&forks[second]);

        printf("Philosopher %d picked up forks %d and %d - now EATING (meal %d)\n",
               id, left, right, meal);
        usleep(10000);
        meals_eaten[id]++;

        pthread_mutex_unlock(&forks[second]);
        pthread_mutex_unlock(&forks[first]);
        printf("Philosopher %d put down forks and is done with meal %d\n", id, meal);
    }
    return NULL;
}

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    pthread_t phils[NUM_PHILOSOPHERS];
    int ids[NUM_PHILOSOPHERS];

    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
        pthread_mutex_init(&forks[i], NULL);

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        ids[i] = i;
        pthread_create(&phils[i], NULL, philosopher, &ids[i]);
    }
    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
        pthread_join(phils[i], NULL);

    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
        pthread_mutex_destroy(&forks[i]);

    printf("\nAll philosophers finished without deadlock.\n");
    printf("Meals eaten per philosopher:\n");
    int ok = 1;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("  Philosopher %d: %d meals\n", i, meals_eaten[i]);
        if (meals_eaten[i] != MEALS_PER_PHILOSOPHER) ok = 0;
    }
    printf("Result: %s\n", ok ? "OK - every philosopher ate exactly the expected number of meals."
                              : "MISMATCH!");
    return 0;
}

/*
 * Sample output (interleaving varies run to run, but it always finishes
 * - that is the point of the fork-ordering rule):
 *
 * Philosopher 0 is THINKING (meal 1/3)
 * Philosopher 1 is THINKING (meal 1/3)
 * ...
 * Philosopher 2 picked up forks 2 and 3 - now EATING (meal 1)
 * ...
 * All philosophers finished without deadlock.
 * Meals eaten per philosopher:
 *   Philosopher 0: 3 meals
 *   ...
 * Result: OK - every philosopher ate exactly the expected number of meals.
 */
