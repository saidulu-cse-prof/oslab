/*
 * PRODUCER-CONSUMER (Bounded Buffer) - Classic Semaphore Solution
 * ---------------------------------------------------------------------
 * One producer thread and one consumer thread share a fixed-size
 * circular buffer. Three synchronization primitives are used, exactly
 * as described in Silberschatz/Galvin:
 *
 *   mutex  - binary semaphore (mutual exclusion on the buffer itself)
 *   empty  - counts free slots, starts at BUFFER_SIZE
 *   full   - counts filled slots, starts at 0
 *
 * The producer waits on "empty" before inserting (blocks if the buffer
 * is full) and signals "full" after inserting. The consumer waits on
 * "full" before removing (blocks if the buffer is empty) and signals
 * "empty" after removing. This is the textbook bounded-buffer problem.
 *
 * Compile: gcc pc_semaphore_bounded.c -o pc_semaphore_bounded -lpthread
 * Run    : ./pc_semaphore_bounded
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 5
#define ITEM_COUNT  12   /* how many items the producer will produce */

static int buffer[BUFFER_SIZE];
static int in = 0, out = 0;              /* circular buffer indices */

static sem_t empty, full;
static pthread_mutex_t mutex;

static int produced_total = 0, consumed_total = 0;

void *producer(void *arg) {
    (void) arg;
    for (int i = 1; i <= ITEM_COUNT; i++) {
        int item = i * 10;                       /* the "item" being produced */

        sem_wait(&empty);                        /* block if buffer is full   */
        pthread_mutex_lock(&mutex);

        buffer[in] = item;
        printf("[Producer] produced %3d -> slot %d\n", item, in);
        in = (in + 1) % BUFFER_SIZE;
        produced_total++;

        pthread_mutex_unlock(&mutex);
        sem_post(&full);                         /* one more full slot        */

        usleep(15000);                           /* simulate production time  */
    }
    return NULL;
}

void *consumer(void *arg) {
    (void) arg;
    for (int i = 1; i <= ITEM_COUNT; i++) {
        sem_wait(&full);                         /* block if buffer is empty  */
        pthread_mutex_lock(&mutex);

        int item = buffer[out];
        printf("[Consumer] consumed %3d <- slot %d\n", item, out);
        out = (out + 1) % BUFFER_SIZE;
        consumed_total++;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);                        /* one more empty slot       */

        usleep(25000);                           /* simulate consumption time */
    }
    return NULL;
}

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    sem_init(&empty, 0, BUFFER_SIZE);   /* all slots start empty */
    sem_init(&full, 0, 0);              /* no slots start full   */
    pthread_mutex_init(&mutex, NULL);

    pthread_t prod_t, cons_t;
    pthread_create(&prod_t, NULL, producer, NULL);
    pthread_create(&cons_t, NULL, consumer, NULL);

    pthread_join(prod_t, NULL);
    pthread_join(cons_t, NULL);

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    printf("\nTotal produced = %d, Total consumed = %d\n", produced_total, consumed_total);
    if (produced_total == consumed_total && produced_total == ITEM_COUNT)
        printf("Result: OK - every item produced was consumed exactly once.\n");
    else
        printf("Result: MISMATCH!\n");

    return 0;
}

/*
 * Sample output (interleaving of PRODUCED/CONSUMED lines varies run to
 * run because the two threads run concurrently, but the final counts
 * always match since the semaphores enforce the bounded-buffer invariant):
 *
 * [Producer] produced  10 -> slot 0
 * [Producer] produced  20 -> slot 1
 * [Consumer] consumed  10 <- slot 0
 * ...
 * Total produced = 12, Total consumed = 12
 * Result: OK - every item produced was consumed exactly once.
 */
