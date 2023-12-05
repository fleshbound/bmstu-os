#ifndef _DAEMONIZE_H
#define _DAEMONIZE_H
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
void daemonize(const char *cmd);
#endif
