/*
 * PAGE REPLACEMENT POLICY (b): LRU (Least Recently Used)
 * ---------------------------------------------------------------------------------
 * Simulates LRU page replacement for a given page-reference string and a
 * fixed number of physical frames.
 *
 * Rule: on a page fault with all frames full, evict whichever RESIDENT
 * page was referenced longest ago (its most recent use is further in
 * the past than every other resident page's most recent use) - unlike
 * FIFO, which only tracks arrival order and ignores use entirely after
 * that.
 *
 * Implementation: for each frame slot we remember "lastUsed[i]", the
 * logical timestamp (a simple increasing counter, one tick per
 * reference) of that slot's most recent access - set on insertion AND
 * refreshed on every later hit. On a fault with no empty frame left,
 * we scan for the slot with the SMALLEST lastUsed value (least
 * recently used) and overwrite it. This is the direct, textbook way to
 * simulate LRU without needing a full stack/counter hardware model.
 *
 * Compile: gcc lru_page_replacement.c -o lru_page_replacement
 * Run    : ./lru_page_replacement
 */

#include <stdio.h>
#include <limits.h>

#define MAX_FRAMES 20
#define MAX_REFS   200

static int frames[MAX_FRAMES];
static int lastUsed[MAX_FRAMES];
static int numFrames;

static int findFrameOf(int page, int filled) {
    for (int i = 0; i < filled; i++)
        if (frames[i] == page) return i;
    return -1;
}

static int findLRUSlot(int filled) {
    int victim = 0;
    for (int i = 1; i < filled; i++)
        if (lastUsed[i] < lastUsed[victim]) victim = i;
    return victim;
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

    for (int i = 0; i < numFrames; i++) { frames[i] = -1; lastUsed[i] = INT_MIN; }

    int filled = 0;
    int faults = 0, hits = 0;
    int clock = 0;            /* logical time, advances once per reference */

    printf("\nRef\tAction\tFrames\n");
    for (int i = 0; i < n; i++) {
        int page = refs[i];
        clock++;
        printf("%d\t", page);

        int slot = findFrameOf(page, filled);
        if (slot != -1) {
            hits++;
            lastUsed[slot] = clock;          /* refresh recency on a hit */
            printf("HIT\t");
            printFrames(filled);
        } else {
            faults++;
            if (filled < numFrames) {
                frames[filled] = page;
                lastUsed[filled] = clock;
                filled++;
                printf("FAULT\t");
                printFrames(filled);
            } else {
                int victim = findLRUSlot(filled);
                printf("FAULT\t(evict %d)\t", frames[victim]);
                frames[victim] = page;
                lastUsed[victim] = clock;
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
 * Expect 12 page faults with LRU on this particular string (versus 15
 * for FIFO on the same string - see fifo_page_replacement.c). LRU does
 * better here because it keeps whichever pages were actually used
 * most recently (e.g. 0, 2, 3 cycle back frequently) instead of just
 * evicting by arrival order.
 */
