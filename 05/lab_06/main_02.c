#define _GNU_SOURCE
#include "apue.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
sigset_t mask;
int fl = 1;
int lockfile(int fd)
{
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return (fcntl(fd, F_SETLK, &fl));
}
int already_running(void)
{
	int fd;
	char buf[16];
	fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
	if (fd == -1)
	{
		syslog(LOG_ERR, "open %s: %s", LOCKFILE, strerror(errno));
		exit(1);
	}
	if (lockfile(fd) == -1)
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			close(fd);
			return 1;
		}
		syslog(LOG_ERR, "lockfile %s: %s", LOCKFILE, strerror(errno));
		exit(1);
	}
	ftruncate(fd, 0);
	sprintf(buf, "%d", getpid());
	write(fd, buf, strlen(buf) + 1);
	return 0;
}
void daemonize(const char *cmd)
{
	int fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;
	umask(0);
	if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
		err_quit("%s: getrlimit", cmd);
	if ((pid = fork()) == -1)
		err_quit("%s: fork", cmd);
	else if (pid != 0)
		exit(0);
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) != 0)
		err_quit("%s: sigaction SIGHUP", cmd);
	if (setsid() == -1)
		err_quit("setsid");
	if (chdir("/") == -1)
		err_quit("%s: chdir /", cmd);
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (size_t i = 0; i < rl.rlim_max; i++)
		close(i);
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0)
	{
		syslog(LOG_ERR, "openlog %d", fd0);
		exit(1);
	}
	if (fd1 != 1)
	{
		syslog(LOG_ERR, "openlog %d", fd1);
		exit(1);
	}
	if (fd2 != 2)
	{
		syslog(LOG_ERR, "openlog %d", fd2);
		exit(1);
	}
}
void sig_handler(int signum)
{
	syslog(LOG_ERR, "catch signal %d", signum);
	fl = 0;
}
void *thr_start(void *arg)
{
	char **str = arg;
	while (fl)
	{
		syslog(LOG_INFO, "pid=%d tid=%d str=%s", getpid(), gettid(), *str);
		sleep(2);
	}
	pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
	int err;
	pthread_t tid[2];
	char *thrstr[2] = { "aaa", "bbb" };
	char *cmd = "02-daemon";
	struct sigaction sa;
	daemonize(cmd);
	if (already_running() != 0)
	{
		syslog(LOG_ERR, "already_running\n");
		exit(1);
	}
	sa.sa_handler = sig_handler;
	if (sigemptyset(&sa.sa_mask) == -1)
	{
		syslog(LOG_ERR, "sigemptyset");
		exit(1);
	}
	if (sigaddset(&sa.sa_mask, SIGTERM) == -1)
	{
		syslog(LOG_ERR, "sigaddset");
		exit(1);
	}
	sa.sa_flags = 0;
	if (sigaction(SIGTERM, &sa, NULL) == -1)
	{
		syslog(LOG_ERR, "sigaction SIGTERM");
		exit(1);
	}
	for (int i = 0; i < 2; i++)
	{
		if ((err = pthread_create(&tid[i], NULL, thr_start, &thrstr[i])) == -1)
		{
			syslog(LOG_ERR, "pthread_create tid=%ld [%d]", tid[i], err);
			exit(1);
		}
	}
	for (int i = 0; i < 2; i++)
	{
		if ((err = pthread_join(tid[i], NULL)) == -1)
		{
			syslog(LOG_ERR, "pthread_join tid=%ld [%d]", tid[i], err);
			exit(1);
		}
	}
	fl = 1;
	long int ttime;
	while (fl)
	{
		ttime = time(NULL);
		syslog(LOG_INFO, "time: %s\n", ctime(&ttime));
		sleep(2);
	}
	return 0;
}
