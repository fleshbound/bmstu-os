#include "apue.h"
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
typedef int my_func_t(const char *, const struct stat *, int);
static my_func_t myfunc;
static int myftw(char *, my_func_t *);
static int dopath(my_func_t *, char *);
int main(int argc, char *argv[])
{
	int ret;
	if (argc != 2)
	err_quit("usage: ftw <start_dir>");
	unsigned long start = clock();
	ret = myftw(argv[1], myfunc);
	unsigned long end = clock() - start;
	printf("\nCHDIR time: %.3f\n", (double) end * 1000000 / CLOCKS_PER_SEC);
	exit(ret);
}
#define FTW_F 1
#define FTW_D 2
#define FTW_DNR 3
#define FTW_NS 4
static int myftw(char *pathname, my_func_t *func)
{
	size_t len;
	return dopath(func, pathname);
}
static int depth;
static int dopath(my_func_t *func, char *curpath)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	int ret;
	char *ptr;
	if (lstat(curpath, &statbuf) < 0)
		return func(curpath, &statbuf, FTW_NS);
	if (S_ISDIR(statbuf.st_mode) == 0)
		return func(curpath, &statbuf, FTW_F);
	if ((ret = func(curpath, &statbuf, FTW_D)) != 0)
		return ret;
	depth++;
	if ((dp = opendir(curpath)) == NULL)
		return func(curpath, &statbuf, FTW_DNR);
	chdir(curpath);
	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;
		if ((ret = dopath(func, dirp->d_name)) != 0)
			break;
	}
	if (closedir(dp) < 0)
		err_ret("closedir %s", curpath);
	chdir("..");
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
				print_sep();
				printf("%s\n", pathname);
			break;
		case FTW_D:
				print_sep();
				printf("%s\n", pathname);
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
