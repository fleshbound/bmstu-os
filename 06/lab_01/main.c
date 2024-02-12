#include <dirent.h>
#include <limits.h>
typedef int my_func_t(const char *, const struct stat *, int);
static my_func_t myfunc;
static int myftw(char *, my_func_t *);
static int dopath(my_func *);
void err_quit(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}
int main(int argc, char *argv[])
{
	int ret;
	if (argc != 2)
		err_quit("usage: ftw <start_dir>");
	ret = myftw(argv[1], myfunc);
	exit(ret);
}
#define FTW_F 1
#define FTW_D 2
#define FTW_DNR 3
#define FTW_NS 4
static char *fullpath;
static int myftw(char *pathname, my_func_t *func)
{
	int len;
	fullpath = path_alloc(&len);
	strncpy(fullpath, pathname, len);
	fullpath[len - 1] = 0;
	return (dopath(func));
}
static int dopath(my_func_t *func)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	int ret;
	char *ptr;
	if (lstat(fullpath, &statbuf) < 0)
		return (func(fullpath, &statbuf, FTW_NS));
	if (S_ISDIR(statbuf.st_mode) == 0)
		return (func(fullpath, &statbuf, FTW_F));
	if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
		return ret;
	ptr = fullpath + strlen(fullpath);
	*ptr++;
	*ptr = 0;
	if ((dp = opendir(fullpath)) == NULL)
		return (func(fullpath, &statbuf, FTW_DNR));
	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
	}
}
