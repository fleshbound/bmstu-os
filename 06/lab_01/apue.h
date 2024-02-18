#ifndef _APUE_H
#define _APUE_H

#define _POSIX_C_SOURCE 200809L

#if defined(SOLARIS)
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 700
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/termios.h>
#if defined(MACOS) || !defined(TIOCGWINSZ)
#include <sys/ioctl.h>
#endif

#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>


#define MAXLINE 4096

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define DIR_MODE (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
	
typedef void Sigfunc(int);

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
	
char *path_alloc(size_t *);
long open_max(void);
	
int set_cloexec(int);
void clr_fl(int, int);
void set_fl(int, int); 	
void pr_exit(int); 
void pr_mask(const char *);
	
Sigfunc *signal_intr(int, Sigfunc *);
	
void daemonize(const char *);
	
void sleep_us(unsigned int);
	
ssize_t readn(int, void *, size_t);
	
ssize_t writen(int, const void *, size_t);

int fd_pipe(int *);
int	recv_fd(int, ssize_t (*func)(int,const void *, size_t));


int send_fd(int, int);
int send_err(int, int,const char *);
int serv_listen(const char *);
int serv_accept(int, uid_t *);
int cli_conn(const char *);
int buf_args(char *, int (*func)(int,char **));

int tty_cbreak(int);
int tty_raw(int);
int tty_reset(int);
void tty_atexit(void);
struct termios *tty_termios(void); 
int ptym_open(char *, int);
int ptys_open(char *);
#ifdef TIOCGWINSZ

pid_t pty_fork(int *, char *, int, const struct termios *,const struct winsize *);
#endif

int lock_reg(int, int, int, off_t, int, off_t); 
#define read_lock(fd, offset, whence, len) \
				   lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define readw_lock(fd, offset, whence, len) \
				   lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define write_lock(fd, offset, whence, len) \
				   lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define writew_lock(fd, offset, whence, len) \
				   lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define un_lock(fd, offset, whence, len) \
				   lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))
			   
pid_t  lock_test(int, int, off_t, int, off_t);
	
#define is_read_lockable(fd, offset, whence, len) \
				   (lock_test((fd), F_RDLCK, (offset), (whence), (len)) == 0)
#define is_write_lockable(fd, offset, whence, len) \
				   (lock_test((fd), F_WRLCK, (offset), (whence), (len)) == 0)
			   
void err_msg(const char *, ...);		   
void err_dump(const char *, ...) __attribute__((noreturn));
void err_quit(const char *, ...) __attribute__((noreturn));
void err_cont(int, const char *, ...);
void err_exit(int, const char *, ...) __attribute__((noreturn));
void err_ret(const char *, ...);
void err_sys(const char *, ...) __attribute__((noreturn));
void log_msg(const char *, ...);


void log_open(const char *, int, int);
void log_quit(const char *, ...) __attribute__((noreturn));
void log_ret(const char *, ...);
void log_sys(const char *, ...) __attribute__((noreturn));
void log_exit(int, const char *, ...) __attribute__((noreturn));

void TELL_WAIT(void);
void TELL_PARENT(pid_t);
void TELL_CHILD(pid_t);
void WAIT_PARENT(void);
void WAIT_CHILD(void);
#endif
