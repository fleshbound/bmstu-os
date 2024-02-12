/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "bakery.h"


void
bakery_prog_1(char *host)
{
	CLIENT *clnt;
	struct BAKERY  *result_1;
	struct BAKERY  getn_1_arg;
	struct BAKERY  *result_2;
	struct BAKERY  proc_1_arg;
	struct BAKERY  *result_3;
	struct BAKERY  wait_1_arg;

	clnt = clnt_create (host, BAKERY_PROG, BAKERY_VER, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
	
	srand(time(NULL));
	double sleep_time = (double)rand() / RAND_MAX * 1000000 * 1.5;
	usleep(sleep_time);
	result_1 = getn_1(&getn_1_arg, clnt);
	if (result_1 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	printf("getn: client pid=%2d, num=%2d\n", result_1->pid, result_1->num);
	usleep((double)rand() / RAND_MAX * 1000000);
	wait_1_arg.num = result_1->num;
	wait_1_arg.pid = result_1->pid;
	result_2 = wait_1(&wait_1_arg, clnt);
	if (result_2 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	sleep(2);
	proc_1_arg.num = wait_1_arg.num;
	proc_1_arg.pid = wait_1_arg.pid;
	result_3 = proc_1(&proc_1_arg, clnt);
	if (result_3 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	printf("proc: client pid=%2d, res=%c (sleep=%.1fs)\n", result_3->pid, result_3->res, sleep_time / 1000000);
	clnt_destroy (clnt);
}


int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	bakery_prog_1 (host);
exit (0);
}