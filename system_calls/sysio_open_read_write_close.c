/*
 * UNIX/LINUX I/O SYSTEM CALLS: open(), write(), read(), close()
 * ---------------------------------------------------------------------------------
 * These four calls are the low-level foundation that <stdio.h> functions
 * like fopen/fread/fwrite/fclose are themselves built on top of. Unlike
 * stdio, they work directly with integer "file descriptors" and raw
 * bytes - no buffering, no text-mode translation.
 *
 * This program:
 *   1. open()s a file for writing, CREATING it if it doesn't exist and
 *      TRUNCATing it if it does (O_CREAT | O_WRONLY | O_TRUNC), with
 *      permission bits 0644 (rw-r--r--) in case it's newly created.
 *   2. write()s a message to it and close()s it.
 *   3. open()s the SAME file again, this time read-only (O_RDONLY).
 *   4. read()s it back in a loop (read() may return fewer bytes than
 *      asked for, so a real program must loop until it returns 0 for
 *      EOF or -1 for error - this program does that correctly).
 *   5. close()s it.
 *
 * Every call's return value is checked, which is the main habit this
 * exercise is meant to teach: unlike stdio's FILE*, these calls report
 * errors as a bare -1 with the reason in errno (inspect with perror()).
 *
 * Compile: gcc sysio_open_read_write_close.c -o sysio_open_read_write_close
 * Run    : ./sysio_open_read_write_close
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FILENAME "syscall_demo.txt"
#define BUF_SIZE 64

int main() {
    const char *message = "Hello from open()/write()! This line was written "
                           "with raw system calls, not fprintf.\n";

    /* ---- WRITE PHASE ---- */
    int fd = open(FILENAME, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open (write) failed");
        exit(1);
    }

    ssize_t to_write = (ssize_t) strlen(message);
    ssize_t written = write(fd, message, to_write);
    if (written == -1) {
        perror("write failed");
        close(fd);
        exit(1);
    }
    printf("Wrote %zd of %zd bytes to \"%s\"\n", written, to_write, FILENAME);

    if (close(fd) == -1) {
        perror("close (after write) failed");
        exit(1);
    }

    /* ---- READ PHASE ---- */
    fd = open(FILENAME, O_RDONLY);
    if (fd == -1) {
        perror("open (read) failed");
        exit(1);
    }

    char buffer[BUF_SIZE];
    ssize_t n;
    printf("\nReading it back in %d-byte chunks:\n", BUF_SIZE - 1);
    int chunk = 0;
    while ((n = read(fd, buffer, BUF_SIZE - 1)) > 0) {
        buffer[n] = '\0';                 /* NUL-terminate just for printf() here */
        printf("  chunk %d (%zd bytes): \"%.*s\"\n", chunk++, n, (int) n, buffer);
    }
    if (n == -1) {
        perror("read failed");
        close(fd);
        exit(1);
    }
    printf("read() returned 0 -> end of file reached.\n");

    if (close(fd) == -1) {
        perror("close (after read) failed");
        exit(1);
    }

    printf("\nDone. (\"%s\" is left on disk so you can inspect it.)\n", FILENAME);
    return 0;
}

/*
 * Sample output:
 *
 * Wrote 78 of 78 bytes to "syscall_demo.txt"
 *
 * Reading it back in 63-byte chunks:
 *   chunk 0 (63 bytes): "Hello from open()/write()! This line was written with raw sys"
 *   chunk 1 (15 bytes): "tem calls, not fprintf.
 * "
 * read() returned 0 -> end of file reached.
 *
 * Done. ("syscall_demo.txt" is left on disk so you can inspect it.)
 */
