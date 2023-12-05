#include "alrun.h"
int lockfile(int fd)
{
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return fcntl(fd, F_SETLK, &fl);
}
int already_running(void)
{
	int fd;
	char buf[16];
	fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
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
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf) + 1);
	return 1;
}
