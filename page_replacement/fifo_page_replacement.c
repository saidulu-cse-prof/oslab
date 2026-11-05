/*
 * PAGE REPLACEMENT POLICY (a): FIFO (First-In First-Out)
 * ---------------------------------------------------------------------------------
 * Simulates FIFO page replacement for a given page-reference string and a
 * fixed number of physical frames.
 *
 * Rule: frames are filled in the order pages first arrive. Once all
 * frames are full, a page fault evicts whichever page has been resident
 * the LONGEST - i.e. the one at the front of a FIFO queue of frame
 * occupants - regardless of how recently or how often it was actually
 * used. (This is FIFO's well-known weakness vs. LRU: a page can be
 * evicted the instant after being referenced, just because it happens
 * to be the oldest resident.)
 *
 * Implementation: frames[] holds the F resident pages (-1 = empty).
 * A FIFO queue is modeled implicitly with a single "nextVictim" index
 * that cycles 0,1,...,F-1,0,1,... - this is exactly equivalent to a
 * real FIFO queue once all frames have been filled once, because the
 * frame that was filled longest ago is always the next one due to be
 * overwritten, in filled order.
 *
 * Compile: gcc fifo_page_replacement.c -o fifo_page_replacement
 * Run    : ./fifo_page_replacement
 */

#include <stdio.h>

#define MAX_FRAMES 20
#define MAX_REFS   200

static int frames[MAX_FRAMES];
static int numFrames;

static int isPresent(int page, int filled) {
    for (int i = 0; i < filled; i++)
        if (frames[i] == page) return 1;
    return 0;
}

static void printFrames(int filled) {
    printf("[ ");
    for (int i = 0; i < numFrames; i++) {
        if (i < filled) printf("%d ", frames[i]);
        else printf("_ ");
    }
    printf("]");
}

int main() {
    int n;

    printf("Enter number of frames: ");
    if (scanf("%d", &numFrames) != 1 || numFrames <= 0 || numFrames > MAX_FRAMES) {
        printf("Invalid number of frames.\n");
        return 1;
    }

    printf("Enter length of reference string: ");
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_REFS) {
        printf("Invalid length.\n");
        return 1;
    }

    int refs[MAX_REFS];
    printf("Enter the %d page references: ", n);
    for (int i = 0; i < n; i++) scanf("%d", &refs[i]);

    for (int i = 0; i < numFrames; i++) frames[i] = -1;

    int filled = 0;          /* how many frames are currently occupied   */
    int nextVictim = 0;      /* FIFO pointer: index of the oldest frame  */
    int faults = 0, hits = 0;

    printf("\nRef\tAction\tFrames\n");
    for (int i = 0; i < n; i++) {
        int page = refs[i];
        printf("%d\t", page);

        if (isPresent(page, filled)) {
            hits++;
            printf("HIT\t");
            printFrames(filled);
        } else {
            faults++;
            if (filled < numFrames) {
                frames[filled] = page;
                filled++;
                printf("FAULT\t");
                printFrames(filled);
            } else {
                printf("FAULT\t(evict %d)\t", frames[nextVictim]);
                frames[nextVictim] = page;
                nextVictim = (nextVictim + 1) % numFrames;   /* advance FIFO queue */
                printFrames(filled);
            }
        }
        printf("\n");
    }

    printf("\nTotal references : %d\n", n);
    printf("Page hits         : %d\n", hits);
    printf("Page faults       : %d\n", faults);
    printf("Hit ratio         : %.2f%%\n", 100.0 * hits / n);

    return 0;
}

/*
 * Sample run (classic textbook reference string, 3 frames):
 *   Reference string: 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
 *   Frames: 3
 *
 * Expect 15 page faults with FIFO on this particular string (versus 12
 * for LRU on the same string - see lru_page_replacement.c - which is
 * exactly the textbook's point: LRU generally performs at least as well
 * as FIFO because it uses more information about actual usage, not just
 * arrival order).
 */
