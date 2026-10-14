/*
 * UNIX/LINUX I/O SYSTEM CALL: lseek()
 * ---------------------------------------------------------------------------------
 * lseek(fd, offset, whence) repositions a file descriptor's read/write
 * cursor without touching the file's contents, enabling RANDOM ACCESS
 * instead of always reading/writing sequentially from wherever the
 * cursor last stopped. "whence" controls what "offset" is relative to:
 *
 *   SEEK_SET - offset is measured from the START of the file
 *   SEEK_CUR - offset is measured from the CURRENT cursor position
 *   SEEK_END - offset is measured from the END of the file
 *
 * This program creates a file of fixed-size "records" (so that record
 * number N always starts at byte N * RECORD_SIZE), then demonstrates:
 *   - jumping directly to an arbitrary record with SEEK_SET
 *   - skipping forward relative to the current position with SEEK_CUR
 *   - finding the file's total size by seeking to offset 0 from
 *     SEEK_END (a common idiom - lseek() with whence=SEEK_END and
 *     offset=0 returns the file size without needing stat()).
 *
 * Compile: gcc sysio_lseek.c -o sysio_lseek
 * Run    : ./sysio_lseek
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FILENAME     "lseek_demo.txt"
#define RECORD_SIZE  16     /* each record is padded/truncated to exactly this many bytes */
#define NUM_RECORDS  5

static void writeRecord(int fd, int index, const char *text) {
    char record[RECORD_SIZE];
    memset(record, ' ', RECORD_SIZE);           /* pad with spaces */
    size_t len = strlen(text);
    if (len > RECORD_SIZE - 1) len = RECORD_SIZE - 1;
    memcpy(record, text, len);
    record[RECORD_SIZE - 1] = '\n';
    write(fd, record, RECORD_SIZE);
    (void) index;
}

int main() {
    int fd = open(FILENAME, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) { perror("open (write) failed"); exit(1); }

    char text[RECORD_SIZE];
    for (int i = 0; i < NUM_RECORDS; i++) {
        snprintf(text, sizeof(text), "Record #%d", i);
        writeRecord(fd, i, text);
    }
    close(fd);
    printf("Wrote %d fixed-size records of %d bytes each to \"%s\".\n\n",
           NUM_RECORDS, RECORD_SIZE, FILENAME);

    fd = open(FILENAME, O_RDONLY);
    if (fd == -1) { perror("open (read) failed"); exit(1); }

    char buf[RECORD_SIZE + 1];

    /* --- SEEK_SET: jump directly to record #1 (do NOT read yet) --- */
    int start_record = 1;
    off_t pos = lseek(fd, start_record * RECORD_SIZE, SEEK_SET);
    printf("lseek(SEEK_SET to record %d) -> cursor at byte %ld\n", start_record, (long) pos);

    /* --- SEEK_CUR: from THERE (not from a read position), skip forward
       two more records' worth of bytes, landing on record #3. Chaining
       seeks like this - rather than seeking right after a read() has
       already moved the cursor past the record just read - is the
       clean way to show SEEK_CUR's "relative to wherever we are now"
       semantics without an off-by-one surprise. */
    int skip_records = 2;
    pos = lseek(fd, skip_records * RECORD_SIZE, SEEK_CUR);
    printf("lseek(SEEK_CUR +%d records) -> cursor at byte %ld (now on record %d)\n",
           skip_records, (long) pos, start_record + skip_records);
    read(fd, buf, RECORD_SIZE);
    buf[RECORD_SIZE] = '\0';
    printf("  Read: \"%.*s\"\n\n", RECORD_SIZE - 1, buf);

    /* --- SEEK_END: find the file size without stat() --- */
    off_t size = lseek(fd, 0, SEEK_END);
    printf("lseek(0, SEEK_END) -> file size = %ld bytes (%ld records of %d bytes)\n",
           (long) size, (long) (size / RECORD_SIZE), RECORD_SIZE);

    close(fd);
    return 0;
}

/*
 * Sample output:
 *
 * Wrote 5 fixed-size records of 16 bytes each to "lseek_demo.txt".
 *
 * lseek(SEEK_SET to record 1) -> cursor at byte 16
 * lseek(SEEK_CUR +2 records) -> cursor at byte 48 (now on record 3)
 *   Read: "Record #3      "
 *
 * lseek(0, SEEK_END) -> file size = 80 bytes (5 records of 16 bytes)
 */
