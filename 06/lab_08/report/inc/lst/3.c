#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PRINT_STAT(action) \
  do { \
    stat("q.txt", &statbuf); \
    fprintf(stdout, action ": inode = %ld, size = %ld bytes, blksize = %ld\n", \
      statbuf.st_ino, statbuf.st_size, \
      statbuf.st_blksize); \
  } while (0);

struct stat statbuf;

int main()
{
  FILE *fs1 = fopen("q.txt", "w");
  FILE *fs2 = fopen("q.txt", "a");
  
  for (char c = 'a'; c <= 'z'; c++)
  {
    if (c % 2)
      fprintf(fs1, "%c", c);
    else
      fprintf(fs2, "%c", c);
    PRINT_STAT("write");
  }

  fclose(fs1);
  PRINT_STAT("fclose fs1");
  fclose(fs2);

  return 0;
}
