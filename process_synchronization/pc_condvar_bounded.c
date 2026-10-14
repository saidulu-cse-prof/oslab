/*
 * PRODUCER-CONSUMER (Bounded Buffer) - Monitor-Style Condition Variable Solution
 * -------------------------------------------------------------------------------------
 * Same problem as pc_semaphore_bounded.c, solved a different way: instead
 * of counting semaphores, a single mutex plus two condition variables
 * ("not_full" and "not_empty") implement a monitor-like structure, the
 * way it would look inside a Monitor's procedures in the textbook.
 *
 *   - The producer locks the mutex, and while the buffer is full it
 *     calls pthread_cond_wait(&not_full, &mutex) - this atomically
 *     releases the mutex and sleeps, exactly like a monitor's
 *     wait() inside a condition.
 *   - After inserting, it signals not_empty to wake a sleeping consumer.
 *   - The consumer is the mirror image, waiting on not_empty and
 *     signalling not_full.
 *
 * This shows that the bounded-buffer problem can be solved with either
 * semaphores or condition variables - condition variables are closer to
 * the monitor construct described in the Process Synchronization chapter.
 *
 * Compile: gcc pc_condvar_bounded.c -o pc_condvar_bounded -lpthread
 * Run    : ./pc_condvar_bounded
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 5
#define ITEM_COUNT  12

static int buffer[BUFFER_SIZE];
static int in = 0, out = 0, count = 0;     /* count = items currently buffered */

static pthread_mutex_t mutex;
static pthread_cond_t  not_full, not_empty;

static int produced_total = 0, consumed_total = 0;

void *producer(void *arg) {
    (void) arg;
    for (int i = 1; i <= ITEM_COUNT; i++) {
        int item = i * 10;

        pthread_mutex_lock(&mutex);
        while (count == BUFFER_SIZE) {             /* buffer full -> wait */
            printf("[Producer] buffer full, waiting...\n");
            pthread_cond_wait(&not_full, &mutex);
        }

        buffer[in] = item;
        printf("[Producer] produced %3d -> slot %d (count=%d)\n", item, in, count + 1);
        in = (in + 1) % BUFFER_SIZE;
        count++;
        produced_total++;

        pthread_cond_signal(&not_empty);           /* wake a waiting consumer */
        pthread_mutex_unlock(&mutex);

        usleep(15000);
    }
    return NULL;
}

void *consumer(void *arg) {
    (void) arg;
    for (int i = 1; i <= ITEM_COUNT; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {                       /* buffer empty -> wait */
            printf("[Consumer] buffer empty, waiting...\n");
            pthread_cond_wait(&not_empty, &mutex);
        }

        int item = buffer[out];
        printf("[Consumer] consumed %3d <- slot %d (count=%d)\n", item, out, count - 1);
        out = (out + 1) % BUFFER_SIZE;
        count--;
        consumed_total++;

        pthread_cond_signal(&not_full);             /* wake a waiting producer */
        pthread_mutex_unlock(&mutex);

        usleep(25000);
    }
    return NULL;
}

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    pthread_t prod_t, cons_t;
    pthread_create(&prod_t, NULL, producer, NULL);
    pthread_create(&cons_t, NULL, consumer, NULL);

    pthread_join(prod_t, NULL);
    pthread_join(cons_t, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    printf("\nTotal produced = %d, Total consumed = %d\n", produced_total, consumed_total);
    if (produced_total == consumed_total && produced_total == ITEM_COUNT)
        printf("Result: OK - every item produced was consumed exactly once.\n");
    else
        printf("Result: MISMATCH!\n");

    return 0;
}

/*
 * Sample output (exact interleaving varies run to run; "waiting..." lines
 * only appear if one side gets far enough ahead of the other to actually
 * fill or drain the 5-slot buffer):
 *
 * [Producer] produced  10 -> slot 0 (count=1)
 * [Consumer] consumed  10 <- slot 0 (count=0)
 * ...
 * Total produced = 12, Total consumed = 12
 * Result: OK - every item produced was consumed exactly once.
 */
