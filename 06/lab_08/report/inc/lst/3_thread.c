#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

struct thread_arg {
    FILE *fs;
    int i;
};

#define PRINT_STAT(action, i) \
    do { \
        stat("q.txt", &statbuf); \
        fprintf(stdout, action " %d: inode = %ld, size = %ld bytes, blksize = %ld\n", \
            i, statbuf.st_ino, statbuf.st_size, \
            statbuf.st_blksize); \
    } while (0);

struct stat statbuf;

void *thread_start(void *arg)
{
    struct thread_arg *targ = arg;

    for (char c = 'a'; c <= 'z'; c++)
        if (c % 2 == targ->i)
        {
            fprintf(targ->fs, "%c", c);
            PRINT_STAT("write", targ->i);
        }
}

int main()
{
    FILE *fs[2] = {fopen("q.txt", "w"),
                   fopen("q.txt", "a")};
    pthread_t thr[2];
    struct thread_arg targ[2];

    for (int i = 0; i < 2; i++)
    {
        targ[i].fs = fs[i];
        targ[i].i = i;

        if (pthread_create(&thr[i], NULL, thread_start, &targ[i]))
        {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < 2; i++)
        if (pthread_join(thr[i], NULL))
        {
            perror("pthread_join");
            return 1;
        }

    fclose(fs[0]);
    PRINT_STAT("fclose fs", 0);
    fclose(fs[1]);
    PRINT_STAT("fclose fs", 1);

    return 0;
}
