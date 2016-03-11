#define ADDTEST_THREADS 0
#define ADDTEST_ITERATIONS 1
#define ADDTEST_YIELD 2
#define ADDTEST_SYNC 3
#include <getopt.h>
#include <pthread.h>

const struct option options[] = {
	{ "threads", required_argument, NULL, ADDTEST_THREADS },
	{ "iterations", required_argument, NULL, ADDTEST_ITERATIONS },
	{ "yield", required_argument, NULL, ADDTEST_YIELD },
	{ "sync", required_argument, NULL, ADDTEST_SYNC },
	{ 0, 0, 0, 0 }
};

void add(long long *pointer, long long value);
void add_mutex(long long *pointer, long long value);
void add_spinlock(long long *pointer, long long value);
void add_cas(long long *pointer, long long value);

int nthreads = 1, niterations = 1;
int opt_yield = 0;
// Choose which add function to run.
void (*addfunc)(long long *pointer, long long value) = add;

pthread_mutex_t mutex;
volatile int lock = 0;
