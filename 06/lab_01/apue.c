#include "apue.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#ifdef PATH_MAX
static long pathmax = PATH_MAX;
#else
static long pathmax = 0;
#endif
static long posix_version = 0;
static long xsi_version = 0;
#define PATH_MAX_GUESS 1024
char *path_alloc(size_t *sizep)
{
	char *ptr;
	size_t size;
	if (posix_version == 0) posix_version = sysconf(_SC_VERSION);
	if (xsi_version == 0) xsi_version = sysconf(_SC_XOPEN_VERSION);
	if (pathmax == 0)
	{
		errno = 0;
		if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0)
		{
			if (errno == 0) pathmax = PATH_MAX_GUESS;
			else err_sys("pathconf");
		}
		else
		{
			pathmax++;
		}
	}
	if ((posix_version < 200112L) && (xsi_version < 4))
		size = pathmax + 1;
	else
		size = pathmax;
	if ((ptr = malloc(size)) == NULL)
		err_sys("malloc");
	if (sizep != NULL)
		*sizep = size;
	return ptr;
}
void err_doit(int errnoflag,int error,const char *fmt,va_list ap)
{
	char buf[MAXLINE];
	vsnprintf(buf, MAXLINE-1, fmt, ap);
	if (errnoflag)
		snprintf(buf+strlen(buf), MAXLINE-strlen(buf)-1, ": %s",strerror(error));
	strcat(buf, "\n");
	fflush(stdout);
	fputs(buf, stderr);
	fflush(NULL);
}
void err_dump(const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	err_doit(1,errno,fmt,ap);
	va_end(ap);
	abort();
	exit(1);
}
void err_sys(const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	err_doit(1,errno,fmt,ap);
	va_end(ap);
	exit(1);
}
void err_ret(const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	err_doit(1,errno,fmt,ap);
	va_end(ap);
}
void err_quit(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}
