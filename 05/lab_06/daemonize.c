#include "daemonize.h"
void daemonize(const char *cmd)
{
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;
	umask(0);
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		err_quit("%s: getrlimit\n", cmd);
	if ((pid = fork()) == -1)
		err_quit("%s: fork\n", cmd);
	else if (pid != 0)
		exit(0);
	setsid();
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) == -1)
		err_quit("%s: sighup\n", cmd);
	if (chdir("/") == -1)
		err_quit("%s: chdir\n", cmd);
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i = 0; i < rl.rlim_max; i++)
		close(i);
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0)
		err_quit("%s open %d\n", fd0);
	if (fd1 != 0)
		err_quit("%s open %d\n", fd1);
	if (fd2 != 0)
		err_quit("%s open %d\n", fd2);
}
