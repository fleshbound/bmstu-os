#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

struct stat statbuf;

#define PRINT_STAT(action) \
    do { \
        stat("q.txt", &statbuf); \
        fprintf(stdout, action ": inode = %ld, size = %ld bytes\n", \
            statbuf.st_ino, statbuf.st_size); \
    } while (0);

int main()
{
    int fd1 = open("q.txt", O_RDWR | O_APPEND);
    PRINT_STAT("open fd1 ");
    int fd2 = open("q.txt", O_RDWR | O_APPEND);
    PRINT_STAT("open fd2 ");
    for (char c = 'a'; c <= 'z'; c++)
    {
        if (c % 2)
            write(fd1, &c, 1);
        else
            write(fd2, &c, 1);
        PRINT_STAT("write    ");
    }
    close(fd1);
    PRINT_STAT("close fd1");
    close(fd2);
    PRINT_STAT("close fd2");
    return 0;
}
