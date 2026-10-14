/*
 * UNIX/LINUX I/O SYSTEM CALL: stat()
 * ---------------------------------------------------------------------------------
 * stat(path, &info) fills a "struct stat" with a file's metadata
 * WITHOUT opening it - size, permission bits, owner, link count,
 * inode number, and the three timestamps (last access / last
 * modification / last status-change). This program creates a demo
 * file, then stat()s it and prints out each field, decoding the
 * permission bits into the familiar "rwxr--r--" style string the way
 * `ls -l` does, and using the S_ISREG/S_ISDIR macros to report the
 * file type.
 *
 * (fstat() is the same idea but takes an already-open file descriptor
 * instead of a path; lstat() is the same as stat() but does not
 * follow a symbolic link if the path names one. All three fill the
 * identical struct stat.)
 *
 * Compile: gcc sysio_stat.c -o sysio_stat
 * Run    : ./sysio_stat
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FILENAME "stat_demo.txt"

static void permString(mode_t mode, char out[11]) {
    out[0] = S_ISDIR(mode) ? 'd' : (S_ISLNK(mode) ? 'l' : '-');
    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';
    out[10] = '\0';
}

int main() {
    /* Create a small demo file so there is always something to stat(),
       even on a fresh machine with no command-line argument given. */
    int fd = open(FILENAME, O_CREAT | O_WRONLY | O_TRUNC, 0640);
    if (fd == -1) { perror("open failed"); exit(1); }
    write(fd, "sample content for stat() to measure\n", 38);
    close(fd);

    struct stat info;
    if (stat(FILENAME, &info) == -1) {
        perror("stat failed");
        exit(1);
    }

    char perms[11];
    permString(info.st_mode, perms);

    printf("stat() results for \"%s\":\n", FILENAME);
    printf("  File type       : %s\n",
           S_ISREG(info.st_mode) ? "regular file" :
           S_ISDIR(info.st_mode) ? "directory"    :
           S_ISLNK(info.st_mode) ? "symbolic link" : "other");
    printf("  Permissions     : %s (octal %o)\n", perms, info.st_mode & 0777);
    printf("  Size            : %ld bytes\n", (long) info.st_size);
    printf("  Inode number    : %ld\n", (long) info.st_ino);
    printf("  Hard link count : %ld\n", (long) info.st_nlink);
    printf("  Owner UID       : %ld\n", (long) info.st_uid);
    printf("  Owner GID       : %ld\n", (long) info.st_gid);
    printf("  Last accessed   : %s", ctime(&info.st_atime));
    printf("  Last modified   : %s", ctime(&info.st_mtime));
    printf("  Last status chg : %s", ctime(&info.st_ctime));

    return 0;
}

/*
 * Sample output (exact inode/uid/gid/timestamps depend on your system):
 *
 * stat() results for "stat_demo.txt":
 *   File type       : regular file
 *   Permissions     : rw-r----- (octal 640)
 *   Size            : 38 bytes
 *   Inode number    : 1234567
 *   Hard link count : 1
 *   Owner UID       : 1000
 *   Owner GID       : 1000
 *   Last accessed   : Wed Oct 14 09:10:00 2026
 *   Last modified   : Wed Oct 14 09:10:00 2026
 *   Last status chg : Wed Oct 14 09:10:00 2026
 */
