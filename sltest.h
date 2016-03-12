#ifndef SLTESTH
#define SLTESTH

#define SLTEST_THREADS 0
#define SLTEST_ITERATIONS 1
#define SLTEST_YIELD 2
#define KEY_MAX_LENGTH 16
#include <getopt.h>
#include <pthread.h>
#include "SortedList.h"

const struct option options[] = {
	{ "threads", required_argument, NULL, SLTEST_THREADS },
	{ "iterations", required_argument, NULL, SLTEST_ITERATIONS },
	{ "yield", required_argument, NULL, SLTEST_YIELD },
	{ 0, 0, 0, 0 }
};

int nthreads = 1, niterations = 1;
int optyield = 0;

SortedList_t list;

#endif
