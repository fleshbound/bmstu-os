/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */
#define _GNU_SOURCE
#define MAX_CLIENT 40
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include "bakery.h"
struct thread_arg
{
	int pid;
	int num;
	int res;
};
time_t raw_time;
struct tm *timeinfo;
pthread_t threads[MAX_CLIENT];
struct thread_arg thr_res[MAX_CLIENT];
bool choosing[MAX_CLIENT] = { 0 };
int number[MAX_CLIENT] = { 0 };
int curr_res = 'a';
int local_pid = 0;

int
bakery(void *arg)
{
	time(&raw_time);
	timeinfo = localtime(&raw_time);
	struct thread_arg *t_arg = arg;
	int i = t_arg->pid;
	printf("start thread tid=%d [client pid=%2d, num=%2d] time=%s", gettid(), i, number[i], asctime(timeinfo));
	for (int j = 0; j < MAX_CLIENT; j++)
	{
		while (choosing[j]);
		while ((number[j] != 0) && (number[j] < number[i] || (number[j] == number[i] && j < i))){
            return -1;
        }
	}
    t_arg->res = curr_res;
	curr_res++;
	if (curr_res > 'z')
		curr_res = 'a';
	time(&raw_time);
    timeinfo = localtime(&raw_time);
	printf("stop  thread tid=%d [client pid=%2d, num=%2d] time=%s", gettid(), i, number[i], asctime(timeinfo));
	number[i] = 0;
	return 0;
}

struct BAKERY *
getn_1_svc(struct BAKERY *argp, struct svc_req *rqstp)
{
	static struct BAKERY  result;
	int i = local_pid;
	local_pid++;
	choosing[i] = true;
	int max_n = 0;
	for (int j = 0; j < MAX_CLIENT; j++)
		if (number[j] > max_n)
			max_n = number[j];
	number[i] = max_n + 1;
	result.pid = i;
	result.num = number[i];
	choosing[i] = false;
	return &result;
}

struct BAKERY *
wait_1_svc(struct BAKERY *argp, struct svc_req *rqstp)
{
	static struct BAKERY  result;
    thr_res[argp->pid].pid = argp->pid;
	// pthread_create(&thr, NULL, bakery, &thr_res[argp->pid]);
	// threads[argp->pid] = thr;
    // sleep(2);
    int err = bakery(&thr_res[argp->pid]);

    if (err != 0) {
        return NULL;
    }

	return &result;
}

struct BAKERY *
proc_1_svc(struct BAKERY *argp, struct svc_req *rqstp)
{
	static struct BAKERY  result;
	result.res = thr_res[argp->pid].res;
	result.pid = argp->pid;
	return &result;
}
