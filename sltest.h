#ifndef SLTESTH
#define SLTESTH

#define SLTEST_THREADS 0
#define SLTEST_ITERATIONS 1
#define SLTEST_YIELD 2
#define SLTEST_SYNC 3
#define KEY_MAX_LENGTH 15
#include <getopt.h>
#include <pthread.h>
#include "SortedList.h"

const struct option options[] = {
	{ "threads", required_argument, NULL, SLTEST_THREADS },
	{ "iterations", required_argument, NULL, SLTEST_ITERATIONS },
	{ "yield", required_argument, NULL, SLTEST_YIELD },
	{ "sync", required_argument, NULL, SLTEST_SYNC },
	{ 0, 0, 0, 0 }
};

int nthreads = 1, niterations = 1;
int opt_yield = 0;

SortedList_t list;

// More convenient to use a flag instead of writing 3 versions of all 4 methods.
int sync = SYNC_NONE;

#endif
