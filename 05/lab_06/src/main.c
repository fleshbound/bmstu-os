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
#include "daemon.h"
sigset_t mask;
void *thr_fn(void *arg)
{
	int err, signo;
	for (;;)
	{
		err = sigwait(&mask, &signo);
		if (err != 0)
		{
			syslog(LOG_ERR, "sigwait");
			exit(1);
		}
		if (signo == SIGHUP)
		{
			syslog(LOG_INFO, "catch SIGHUP getlogin=%s", getlogin());
			break;
		}
		else if (signo == SIGTERM)
		{
			syslog(LOG_INFO, "catch SIGTERM");
			exit(0);
		}
		else
		{
			syslog(LOG_INFO, "catch signal %d\n", signo);
		}
	}
	return 0;
}
int main(int argc, char *argv[])
{
	int err;
	pthread_t tid;
	char *cmd = "test-daemon";
	struct sigaction sa;
	daemonize(cmd);
	if (already_running() != 0)
	{
		syslog(LOG_ERR, "already_running\n");
		exit(1);
	}
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) == -1)
		err_quit("%s: sigaction SIGHUP", cmd);
	sigfillset(&mask);
	if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
		err_exit(err, "SIG_BLOCK");
	if ((err = pthread_create(&tid, NULL, thr_fn, NULL)) != 0)
		err_exit(err, "pthread_create");
	syslog(LOG_WARNING, "check done\n");
	long int ttime;
	while (1)
	{
		ttime = time(NULL);
		syslog(LOG_INFO, "time: %s\n", ctime(&ttime));
		sleep(3);
	}
}
