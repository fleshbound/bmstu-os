#include "apue.h"
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#define ITER 1000
typedef int my_func_t(const char *, const struct stat *, int);
static my_func_t myfunc;
static int myftw(char *, my_func_t *);
static int dopath(my_func_t *);
int debug = 0;
int main(int argc, char *argv[])
{
	int ret;
	if (argc != 2)
		err_quit("usage: ftw <start_dir>");
	ret = myftw(argv[1], myfunc);
	debug = 1;
	double sum_time = 0;
	for (int i = 0; i < ITER; i++)
	{
		unsigned long start = clock();
		ret = myftw(argv[1], myfunc);
		unsigned long end = clock() - start;
		sum_time += (double) end * 1000000 / CLOCKS_PER_SEC;
	}
	printf("\nCHDIR time: %.3f\n", sum_time / (double) ITER);
	exit(ret);
}
#define FTW_F 1
#define FTW_D 2
#define FTW_DNR 3
#define FTW_NS 4
static char *fullpath;
static int myftw(char *pathname, my_func_t *func)
{
	size_t len;
	fullpath = path_alloc(&len);
	strncpy(fullpath, pathname, len);
	fullpath[len - 1] = 0;
	return dopath(func);
}
static int depth;
static int dopath(my_func_t *func)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	int ret;
	char *ptr;
	if (lstat(fullpath, &statbuf) < 0)
		return func(fullpath, &statbuf, FTW_NS);
	if (S_ISDIR(statbuf.st_mode) == 0)
		return func(fullpath, &statbuf, FTW_F);
	if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
		return ret;
	depth++;
	ptr = fullpath + strlen(fullpath);
	*ptr++ = '/';
	*ptr = 0;
	if ((dp = opendir(fullpath)) == NULL)
		return func(fullpath, &statbuf, FTW_DNR);
	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;
		strcpy(ptr, dirp->d_name);
		if ((ret = dopath(func)) != 0)
			break;
	}
	ptr[-1] = 0;
	if (closedir(dp) < 0)
		err_ret("closedir %s", fullpath);
	depth--;
	return ret;
}
void print_sep(void)
{
	for (int i = 0; i < depth; i++)
		printf("* ");
}
static int myfunc(const char *pathname, const struct stat *statbuf, int type)
{
	switch (type)
	{
		case FTW_F:
			if (!debug)
			{
				print_sep();
				printf("%s\n", pathname);
			}
			break;
		case FTW_D:
			if (!debug)
			{
				print_sep();
				printf("%s\n", pathname);
			}
			break;
		case FTW_DNR:
			err_ret("no access %s", pathname);
			break;
		case FTW_NS:
			err_ret("stat %s", pathname);
			break;
		default:
			err_dump("unknown type %d of %s", type, pathname);
	}
	return 0;
}
