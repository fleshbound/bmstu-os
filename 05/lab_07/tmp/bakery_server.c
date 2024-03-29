/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include "bakery.h"

struct thread_arg
{
	int id;
	int num;
	char sym;
};
pthread_t threads[25];
struct thread_arg thr_res[25] = { 0 };

bool choosing[25] = { 0 };
int number[25] = { 0 };
char curr_s = 'a';
int curr_n = 0;
int create_id = 0;
int join_id = 0;

void *
bakery(void *arg)
{
	struct thread_arg *t_arg = arg;
	int i = t_arg->id;
	printf("start thread tid=%d, num=%d\n", gettid(), number[i]);
	for (int j = 0; j < 25; j++)
	{
		while (choosing[i]);
		while ((number[j] != 0) && (number[j] < number[i] || (number[j] == number[i] && j < i)));
	}
	t_arg->sym = curr_s;
	curr_s++;
	sleep(2);
	printf("stop thread tid=%d, num=%d\n", gettid(), number[i]);
	number[i] = 0;
	return 0;
}

struct BAKERY *
getn_1_svc(struct BAKERY *argp, struct svc_req *rqstp)
{
	static struct BAKERY  result;
	int i = curr_n;
	curr_n++;
	choosing[i] = true;
	int max_n = -1;
	for (int j = 0; j < 25; j++)
		if (number[i] > max_n)
			max_n = number[i];
	number[i] = max_n + 1;
	choosing[i] = false;
	result.pid = i;
	result.num = number[i];
	return &result;
}

struct BAKERY *
proc_1_svc(struct BAKERY *argp, struct svc_req *rqstp)
{
	static struct BAKERY  result;
	pthread_join(threads[join_id], NULL);
	result->res = argp->res;
	result->puid = argp->pid;
	join_id++;
	return &result;
}

struct BAKERY *
wait_1_svc(struct BAKERY *argp, struct svc_req *rqstp)
{
	static struct BAKERY  result;
	pthread_t thr;
	thr_res[create_id].id = argp->pid;
	pthread_create(&thr, NULL, bakery, &thr_res[create_id]);
	threads[create_id] = thr;
	create_id++;
	return &result;
}
