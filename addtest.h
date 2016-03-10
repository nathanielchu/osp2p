#define ADDTEST_THREADS 0
#define ADDTEST_ITERATIONS 1
#define ADDTEST_YIELD 2
#include <getopt.h>

const struct option options[] = {
	{ "threads", required_argument, NULL, ADDTEST_THREADS },
	{ "iterations", required_argument, NULL, ADDTEST_ITERATIONS },
	{ "yield", required_argument, NULL, ADDTEST_YIELD },
	{ 0, 0, 0, 0 }
};

int opt_yield;
