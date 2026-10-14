/*
 * DINING PHILOSOPHERS - Monitor-Style Solution (Silberschatz/Galvin)
 * ---------------------------------------------------------------------------------
 * This follows the classic monitor solution from the textbook almost
 * line for line, implemented with a pthread mutex (the monitor's
 * implicit lock) and one condition variable per philosopher (standing
 * in for the monitor's "self[i]" condition array):
 *
 *   enum {THINKING, HUNGRY, EATING} state[N];
 *
 *   pickup(i):
 *       state[i] = HUNGRY
 *       test(i)                      // try to let i eat immediately
 *       if state[i] != EATING: wait on self[i]   // else block
 *
 *   putdown(i):
 *       state[i] = THINKING
 *       test(left of i)              // neighbors might now be able to eat
 *       test(right of i)
 *
 *   test(i):
 *       if state[i]==HUNGRY and neighbors are NOT EATING:
 *           state[i] = EATING
 *           signal self[i]           // wake i if it was waiting
 *
 * Because a philosopher only enters EATING when test() has confirmed
 * neither neighbor is currently eating, and all state reads/writes are
 * serialized by the one monitor mutex, two neighbors can never eat at
 * the same time (mutual exclusion on shared forks) and - because
 * test() is also re-run by both neighbors whenever either of them
 * finishes eating - a hungry philosopher is never forgotten (no
 * starvation-by-bookkeeping, no deadlock, since nobody ever blocks
 * while holding a fork: forks are never modeled as separate locks at
 * all here, only as the state array).
 *
 * Compile: gcc dp_monitor_state.c -o dp_monitor_state -lpthread
 * Run    : ./dp_monitor_state
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_PHILOSOPHERS 5
#define MEALS_PER_PHILOSOPHER 3

typedef enum { THINKING, HUNGRY, EATING } State;

static State state[NUM_PHILOSOPHERS];
static pthread_mutex_t monitor_mutex;          /* the monitor's single lock       */
static pthread_cond_t  self_cond[NUM_PHILOSOPHERS]; /* one condition per philosopher */
static int meals_eaten[NUM_PHILOSOPHERS];

static int left_of(int i)  { return (i + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS; }
static int right_of(int i) { return (i + 1) % NUM_PHILOSOPHERS; }

/* Must be called while holding monitor_mutex. */
static void test(int i) {
    if (state[i] == HUNGRY &&
        state[left_of(i)]  != EATING &&
        state[right_of(i)] != EATING) {
        state[i] = EATING;
        printf("Philosopher %d: both neighbors free -> now EATING\n", i);
        pthread_cond_signal(&self_cond[i]);
    }
}

static void pickup(int i) {
    pthread_mutex_lock(&monitor_mutex);

    state[i] = HUNGRY;
    printf("Philosopher %d is HUNGRY\n", i);
    test(i);                              /* maybe we can eat right away */
    while (state[i] != EATING) {
        printf("Philosopher %d must wait - a neighbor is eating\n", i);
        pthread_cond_wait(&self_cond[i], &monitor_mutex);
    }

    pthread_mutex_unlock(&monitor_mutex);
}

static void putdown(int i) {
    pthread_mutex_lock(&monitor_mutex);

    state[i] = THINKING;
    printf("Philosopher %d finished EATING -> THINKING\n", i);
    test(left_of(i));                     /* neighbors may now be able to eat */
    test(right_of(i));

    pthread_mutex_unlock(&monitor_mutex);
}

void *philosopher(void *arg) {
    int id = *(int *) arg;

    for (int meal = 1; meal <= MEALS_PER_PHILOSOPHER; meal++) {
        printf("Philosopher %d is THINKING (meal %d/%d)\n", id, meal, MEALS_PER_PHILOSOPHER);
        usleep(5000);

        pickup(id);
        usleep(10000);                    /* eating */
        meals_eaten[id]++;
        putdown(id);
    }
    return NULL;
}

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    pthread_mutex_init(&monitor_mutex, NULL);
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        state[i] = THINKING;
        pthread_cond_init(&self_cond[i], NULL);
    }

    pthread_t phils[NUM_PHILOSOPHERS];
    int ids[NUM_PHILOSOPHERS];
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        ids[i] = i;
        pthread_create(&phils[i], NULL, philosopher, &ids[i]);
    }
    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
        pthread_join(phils[i], NULL);

    pthread_mutex_destroy(&monitor_mutex);
    for (int i = 0; i < NUM_PHILOSOPHERS; i++)
        pthread_cond_destroy(&self_cond[i]);

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
 * Sample output (interleaving varies; the key guarantee is that you will
 * never see two ADJACENT philosophers both print "now EATING" without an
 * intervening "finished EATING" from one of them):
 *
 * Philosopher 0 is THINKING (meal 1/3)
 * Philosopher 0 is HUNGRY
 * Philosopher 0: both neighbors free -> now EATING
 * ...
 * Philosopher 0 finished EATING -> THINKING
 * Result: OK - every philosopher ate exactly the expected number of meals.
 */
