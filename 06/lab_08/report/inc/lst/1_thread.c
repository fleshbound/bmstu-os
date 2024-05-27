#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

struct thread_arg {
    FILE *fs;
    int i;
};

void *thread_start(void *arg)
{
    struct thread_arg *targ = arg;
    char c;

    while (fscanf(targ->fs, "%c", &c) == 1)
        fprintf(stdout, "thread %d: %c\n", targ->i, c);

    return NULL;
}

int main()
{
    int fd = open("alphabet.txt", O_RDONLY);
    FILE *fs[2] = {fdopen(fd, "r"),
                   fdopen(fd, "r")};
    char buff[2][20];
    
    setvbuf(fs[0], buff[0], _IOFBF, 20);
    setvbuf(fs[1], buff[1], _IOFBF, 20);
    
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

    return 0;
}
