/*
 * DINING PHILOSOPHERS - "Room" Semaphore Solution (limit N-1 at the table)
 * ---------------------------------------------------------------------------------------
 * Each fork is still its own mutex, but before a philosopher may even
 * try to pick up a fork, it must first acquire a seat in a counting
 * semaphore "room" that is initialized to NUM_PHILOSOPHERS - 1 instead
 * of NUM_PHILOSOPHERS.
 *
 * Why this prevents deadlock: the classic deadlock requires ALL N
 * philosophers to simultaneously hold exactly one fork each while
 * waiting for their other fork (a full circular wait around the whole
 * table). By allowing at most N-1 philosophers to be "at the table"
 * (i.e. attempting to acquire forks) at any moment, at least one
 * philosopher is always kept out entirely - which means at least one
 * fork is always available to whichever of that excluded philosopher's
 * two neighbors needs it, breaking the cycle before it can form. Each
 * philosopher still picks up its lower-numbered fork first as a second
 * layer of safety, but the "room" semaphore alone is already enough to
 * make deadlock impossible with N >= 2.
 *
 * Compile: gcc dp_waiter_semaphore.c -o dp_waiter_semaphore -lpthread
 * Run    : ./dp_waiter_semaphore
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_PHILOSOPHERS 5
#define MEALS_PER_PHILOSOPHER 3

static pthread_mutex_t forks[NUM_PHILOSOPHERS];
static sem_t room;                 /* at most NUM_PHILOSOPHERS-1 may be seated */
static int meals_eaten[NUM_PHILOSOPHERS];

void *philosopher(void *arg) {
    int id = *(int *) arg;
    int left  = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    int first  = (left < right) ? left  : right;
    int second = (left < right) ? right : left;

    for (int meal = 1; meal <= MEALS_PER_PHILOSOPHER; meal++) {
        printf("Philosopher %d is THINKING (meal %d/%d)\n", id, meal, MEALS_PER_PHILOSOPHER);
        usleep(5000);

        printf("Philosopher %d is HUNGRY, waiting for a seat at the table\n", id);
        sem_wait(&room);                       /* <= N-1 philosophers past this point */

        pthread_mutex_lock(&forks[first]);
        pthread_mutex_lock(&forks[second]);

        printf("Philosopher %d picked up forks %d and %d - now EATING (meal %d)\n",
               id, left, right, meal);
        usleep(10000);
        meals_eaten[id]++;

        pthread_mutex_unlock(&forks[second]);
        pthread_mutex_unlock(&forks[first]);

        sem_post(&room);                       /* leave the table, free a seat */
        printf("Philosopher %d put down forks and left the table (meal %d done)\n", id, meal);
    }
    return NULL;
}

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    pthread_t phils[NUM_PHILOSOPHERS];
    int ids[NUM_PHILOSOPHERS];

    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
        pthread_mutex_init(&forks[i], NULL);
    sem_init(&room, 0, NUM_PHILOSOPHERS - 1);

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        ids[i] = i;
        pthread_create(&phils[i], NULL, philosopher, &ids[i]);
    }
    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
        pthread_join(phils[i], NULL);

    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
        pthread_mutex_destroy(&forks[i]);
    sem_destroy(&room);

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
 * Sample output (interleaving varies run to run, but with only N-1 = 4
 * philosophers ever allowed past the "room" semaphore at once, the
 * system always finishes cleanly):
 *
 * Philosopher 0 is THINKING (meal 1/3)
 * Philosopher 0 is HUNGRY, waiting for a seat at the table
 * Philosopher 0 picked up forks 0 and 1 - now EATING (meal 1)
 * ...
 * Result: OK - every philosopher ate exactly the expected number of meals.
 */
