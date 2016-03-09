#define ADDTEST_THREADS 0
#define ADDTEST_ITERATIONS 1
#include <getopt.h>

const struct option options[] = {
	{ "threads", required_argument, NULL, ADDTEST_THREADS },
	{ "iterations", required_argument, NULL, ADDTEST_ITERATIONS },
	{ 0, 0, 0, 0 }
};
