/*
 * PRODUCER-CONSUMER (Bounded Buffer) - MULTIPLE Producers and Consumers
 * ---------------------------------------------------------------------------------
 * Generalizes the single-producer/single-consumer case to NUM_PRODUCERS
 * producer threads and NUM_CONSUMERS consumer threads sharing one
 * bounded circular buffer. Synchronization still uses the classic
 * empty/full counting semaphores plus a mutex, but now the mutex is
 * what matters most: with several producers (or several consumers)
 * racing each other, it must serialize access to the shared "in"/"out"
 * indices, not just producer-vs-consumer access.
 *
 * Each produced item is tagged with the id of the producer that made
 * it and a per-producer sequence number, so at the end we can verify
 * that every single item produced was consumed exactly once - i.e. no
 * item was lost or double-counted by a race condition.
 *
 * Compile: gcc pc_multi_producers_consumers.c -o pc_multi_producers_consumers -lpthread
 * Run    : ./pc_multi_producers_consumers
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE      6
#define NUM_PRODUCERS    3
#define NUM_CONSUMERS    2
#define ITEMS_PER_PRODUCER 5
#define TOTAL_ITEMS      (NUM_PRODUCERS * ITEMS_PER_PRODUCER)

typedef struct {
    int producer_id;
    int seq;             /* this producer's i-th item, 1..ITEMS_PER_PRODUCER */
} Item;

static Item buffer[BUFFER_SIZE];
static int in = 0, out = 0;

static sem_t empty, full;
static pthread_mutex_t buf_mutex;        /* protects buffer + in/out            */
static pthread_mutex_t count_mutex;      /* protects produced_total/consumed_total */

static int produced_total = 0, consumed_total = 0;
static int items_remaining = TOTAL_ITEMS;   /* claim ticket pool for consumers */

/* seen[p][s] marks that producer p's item #s was consumed - lets us
   verify afterwards that every item was delivered exactly once. */
static int seen[NUM_PRODUCERS][ITEMS_PER_PRODUCER + 1];
static pthread_mutex_t seen_mutex;

void *producer(void *arg) {
    int id = *(int *) arg;

    for (int seq = 1; seq <= ITEMS_PER_PRODUCER; seq++) {
        Item item = { .producer_id = id, .seq = seq };

        sem_wait(&empty);
        pthread_mutex_lock(&buf_mutex);

        buffer[in] = item;
        printf("[Producer %d] produced item #%d -> slot %d\n", id, seq, in);
        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buf_mutex);
        sem_post(&full);

        pthread_mutex_lock(&count_mutex);
        produced_total++;
        pthread_mutex_unlock(&count_mutex);

        usleep(10000);
    }
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *) arg;

    for (;;) {
        /* Claim one of the TOTAL_ITEMS "tickets" before blocking on &full.
           Exactly TOTAL_ITEMS tickets exist, and exactly TOTAL_ITEMS items
           will ever be posted to &full by the producers, so a consumer
           that successfully claims a ticket is guaranteed a real item is
           coming - this avoids any race where a consumer could be woken
           with nothing left to consume. */
        pthread_mutex_lock(&count_mutex);
        if (items_remaining == 0) {                 /* no tickets left - stop */
            pthread_mutex_unlock(&count_mutex);
            break;
        }
        items_remaining--;
        pthread_mutex_unlock(&count_mutex);

        sem_wait(&full);
        pthread_mutex_lock(&buf_mutex);

        Item item = buffer[out];
        printf("[Consumer %d] consumed producer %d's item #%d <- slot %d\n",
               id, item.producer_id, item.seq, out);
        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buf_mutex);
        sem_post(&empty);

        pthread_mutex_lock(&seen_mutex);
        seen[item.producer_id][item.seq]++;
        pthread_mutex_unlock(&seen_mutex);

        pthread_mutex_lock(&count_mutex);
        consumed_total++;
        pthread_mutex_unlock(&count_mutex);

        usleep(15000);
    }
    return NULL;
}

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&buf_mutex, NULL);
    pthread_mutex_init(&count_mutex, NULL);
    pthread_mutex_init(&seen_mutex, NULL);

    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS], consumer_ids[NUM_CONSUMERS];

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) pthread_join(producers[i], NULL);
    for (int i = 0; i < NUM_CONSUMERS; i++) pthread_join(consumers[i], NULL);

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&buf_mutex);
    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&seen_mutex);

    printf("\nTotal produced = %d, Total consumed = %d\n", produced_total, consumed_total);

    int ok = (produced_total == TOTAL_ITEMS && consumed_total == TOTAL_ITEMS);
    for (int p = 1; p <= NUM_PRODUCERS && ok; p++)
        for (int s = 1; s <= ITEMS_PER_PRODUCER; s++)
            if (seen[p][s] != 1) ok = 0;   /* lost (0) or duplicated (>1) */

    if (ok)
        printf("Result: OK - all %d items from %d producers were each consumed exactly once.\n",
               TOTAL_ITEMS, NUM_PRODUCERS);
    else
        printf("Result: MISMATCH - an item was lost or double-delivered!\n");

    return 0;
}

/*
 * Sample output (interleaving across the 3 producer and 2 consumer
 * threads varies run to run, but the final verification always passes):
 *
 * [Producer 1] produced item #1 -> slot 0
 * [Producer 2] produced item #1 -> slot 1
 * [Consumer 1] consumed producer 1's item #1 <- slot 0
 * ...
 * Total produced = 15, Total consumed = 15
 * Result: OK - all 15 items from 3 producers were each consumed exactly once.
 */
