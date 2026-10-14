/*
 * BANKER'S ALGORITHM - Deadlock AVOIDANCE
 * ---------------------------------------------------------------------------------
 * Implements both halves of the Banker's Algorithm as described in
 * Silberschatz/Galvin's "Operating System Concepts":
 *
 *   1. SAFETY ALGORITHM (section 8.6.2)
 *      Decides whether the CURRENT allocation state is "safe" - i.e.
 *      whether there exists at least one order in which every process
 *      could still finish, even if every process immediately asked for
 *      its full remaining Need. If so, that order is printed as the
 *      "safe sequence".
 *
 *   2. RESOURCE-REQUEST ALGORITHM (section 8.6.3)
 *      Decides what to do when a process asks for more resources:
 *        - If the request exceeds that process's declared Need, it's
 *          an error (the process asked for more than it promised it
 *          would ever need).
 *        - If the request exceeds what is currently Available, the
 *          process must simply wait (not a deadlock - the resources
 *          may free up later).
 *        - Otherwise, the request is PRETEND-granted and the safety
 *          algorithm is re-run on the resulting state. If that
 *          hypothetical state is still safe, the request is granted
 *          for real. If it would be unsafe, the pretend-allocation is
 *          rolled back and the process must wait instead - this is the
 *          whole point of the algorithm: it never lets the system
 *          enter an unsafe state, so deadlock is AVOIDED, not just
 *          detected after the fact.
 *
 * Resources here are general "resource types" with possibly more than
 * one instance each (e.g. 3 instances of a resource type), matching
 * the textbook's model - not just single mutex-like resources.
 *
 * Compile: gcc bankers_algorithm.c -o bankers_algorithm
 * Run    : ./bankers_algorithm
 */

#include <stdio.h>
#include <string.h>

#define MAX_PROCESSES 20
#define MAX_RESOURCES 10

static int n, m;                                   /* processes, resource types */
static int available[MAX_RESOURCES];
static int maxClaim[MAX_PROCESSES][MAX_RESOURCES];
static int allocation[MAX_PROCESSES][MAX_RESOURCES];
static int need[MAX_PROCESSES][MAX_RESOURCES];

static void calculateNeed(void) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            need[i][j] = maxClaim[i][j] - allocation[i][j];
}

static void printMatrix(const char *label, int mat[MAX_PROCESSES][MAX_RESOURCES]) {
    printf("%s\n", label);
    printf("        ");
    for (int j = 0; j < m; j++) printf("R%-3d", j);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("  P%-3d  ", i);
        for (int j = 0; j < m; j++) printf("%-4d", mat[i][j]);
        printf("\n");
    }
}

static void printAvailable(void) {
    printf("Available: ");
    for (int j = 0; j < m; j++) printf("%d ", available[j]);
    printf("\n");
}

static void printState(void) {
    printMatrix("Allocation:", allocation);
    printMatrix("Max:", maxClaim);
    printMatrix("Need:", need);
    printAvailable();
}

/*
 * SAFETY ALGORITHM
 * Work[]   - a working copy of Available, grown as processes "finish"
 * Finish[] - whether each process has been proven able to complete
 * On success, sequence[] holds one valid completion order and the
 * function returns 1; otherwise it returns 0 (state is unsafe).
 */
static int isSafe(int sequence[MAX_PROCESSES]) {
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};
    memcpy(work, available, sizeof(int) * m);

    int completed = 0;
    while (completed < n) {
        int found = 0;
        for (int i = 0; i < n; i++) {
            if (finish[i]) continue;

            int canProceed = 1;
            for (int j = 0; j < m; j++) {
                if (need[i][j] > work[j]) { canProceed = 0; break; }
            }
            if (!canProceed) continue;

            /* Process i could get everything it might still ask for;
               simulate it running to completion and releasing its
               resources back into Work. */
            for (int j = 0; j < m; j++) work[j] += allocation[i][j];
            finish[i] = 1;
            sequence[completed++] = i;
            found = 1;
        }
        if (!found) break;   /* no process could proceed this pass -> stuck */
    }
    return completed == n;
}

static void reportSafety(const char *when) {
    int sequence[MAX_PROCESSES];
    printf("\n--- Safety check %s ---\n", when);
    if (isSafe(sequence)) {
        printf("State is SAFE. A safe sequence is: ");
        for (int i = 0; i < n; i++) printf("P%d%s", sequence[i], (i < n - 1) ? " -> " : "\n");
    } else {
        printf("State is UNSAFE - deadlock could occur!\n");
    }
}

/*
 * RESOURCE-REQUEST ALGORITHM
 * Returns:
 *   0 - request GRANTED (state updated for real)
 *   1 - process must WAIT: request exceeds currently available resources
 *   2 - request DENIED: granting it would leave the system unsafe
 *       (pretend-allocation rolled back, state unchanged)
 *   3 - ERROR: request exceeds the process's declared Need
 */
static int requestResources(int pid, int request[MAX_RESOURCES], int sequence[MAX_PROCESSES]) {
    for (int j = 0; j < m; j++) {
        if (request[j] > need[pid][j]) return 3;
    }
    for (int j = 0; j < m; j++) {
        if (request[j] > available[j]) return 1;
    }

    /* Pretend to allocate */
    for (int j = 0; j < m; j++) {
        available[j]      -= request[j];
        allocation[pid][j] += request[j];
        need[pid][j]       -= request[j];
    }

    if (isSafe(sequence)) {
        return 0;                                  /* keep the change - granted */
    }

    /* Unsafe - roll back */
    for (int j = 0; j < m; j++) {
        available[j]      += request[j];
        allocation[pid][j] -= request[j];
        need[pid][j]       += request[j];
    }
    return 2;
}

static void runRequestDemo(int pid, int request[MAX_RESOURCES]) {
    int sequence[MAX_PROCESSES];

    printf("\n>>> P%d requests ( ", pid);
    for (int j = 0; j < m; j++) printf("%d ", request[j]);
    printf(")\n");

    int result = requestResources(pid, request, sequence);
    switch (result) {
        case 0:
            printf("GRANTED - request satisfied immediately, new state is safe.\n");
            printf("Safe sequence from here: ");
            for (int i = 0; i < n; i++) printf("P%d%s", sequence[i], (i < n - 1) ? " -> " : "\n");
            printState();
            break;
        case 1:
            printf("MUST WAIT - not enough units currently available for this request.\n");
            break;
        case 2:
            printf("DENIED - granting this would leave the system in an UNSAFE state.\n");
            printf("(allocation rolled back; P%d must wait)\n", pid);
            break;
        case 3:
            printf("ERROR - this request exceeds what P%d declared as its maximum need.\n", pid);
            break;
    }
}

int main() {
    printf("Enter number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_PROCESSES) { printf("Invalid n.\n"); return 1; }

    printf("Enter number of resource types: ");
    if (scanf("%d", &m) != 1 || m <= 0 || m > MAX_RESOURCES) { printf("Invalid m.\n"); return 1; }

    printf("Enter the Available vector (%d values): ", m);
    for (int j = 0; j < m; j++) scanf("%d", &available[j]);

    for (int i = 0; i < n; i++) {
        printf("Enter Max claim for P%d (%d values): ", i, m);
        for (int j = 0; j < m; j++) scanf("%d", &maxClaim[i][j]);

        printf("Enter current Allocation for P%d (%d values): ", i, m);
        for (int j = 0; j < m; j++) scanf("%d", &allocation[i][j]);
    }

    calculateNeed();

    printf("\n=== Initial State ===\n");
    printState();
    reportSafety("on initial state");

    printf("\nHow many resource requests would you like to simulate? ");
    int numRequests = 0;
    scanf("%d", &numRequests);

    for (int r = 0; r < numRequests; r++) {
        int pid, request[MAX_RESOURCES];
        printf("\nRequest #%d - enter requesting process id (0-%d): ", r + 1, n - 1);
        scanf("%d", &pid);
        if (pid < 0 || pid >= n) { printf("Invalid process id, skipping.\n"); continue; }

        printf("Enter request vector (%d values): ", m);
        for (int j = 0; j < m; j++) scanf("%d", &request[j]);

        runRequestDemo(pid, request);
    }

    return 0;
}

/*
 * Sample run, using the classic 5-process / 3-resource-type example
 * from Silberschatz, Galvin & Gagne (resources A, B, C with 10, 5 and
 * 7 total instances):
 *
 *   n = 5
 *   m = 3
 *   Available: 3 3 2
 *   P0  Max: 7 5 3   Allocation: 0 1 0
 *   P1  Max: 3 2 2   Allocation: 2 0 0
 *   P2  Max: 9 0 2   Allocation: 3 0 2
 *   P3  Max: 2 2 2   Allocation: 2 1 1
 *   P4  Max: 4 3 3   Allocation: 0 0 2
 *
 * -> Need works out to:
 *      P0: 7 4 3   P1: 1 2 2   P2: 6 0 0   P3: 0 1 1   P4: 4 3 1
 * -> Initial state is SAFE (e.g. safe sequence P1 -> P3 -> P4 -> P0 -> P2,
 *    or P1 -> P3 -> P4 -> P2 -> P0 - any order the algorithm finds is
 *    equally valid proof of safety).
 *
 * Then simulate 3 requests to see all three outcomes:
 *   1) P1 requests (1 0 2)  -> GRANTED  (fits in Available, result stays safe)
 *   2) P4 requests (3 3 0)  -> MUST WAIT (3 exceeds the 2 units of resource A
 *                                          now available after request #1)
 *   3) P0 requests (0 2 0)  -> DENIED    (fits in what's available, but
 *                                          granting it would leave no safe
 *                                          completion order - classic
 *                                          textbook illustration of why the
 *                                          safety check matters even when a
 *                                          request "fits")
 */
