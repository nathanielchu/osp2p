#define _GNU_SOURCE
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "addtest.h"

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield)
		pthread_yield();
	*pointer = sum;
}

typedef struct threadarg {
	int niterations;

	long long *pointer;

	// Choose which add function to run.
	void (*addfunc)(long long *pointer, long long value);
} threadarg_t;

void *process(void *arg) {
	threadarg_t *threadarg = arg;
	int i;
	// Add 1.
	for (i = 0; i < threadarg->niterations; i++) {
		threadarg->addfunc(threadarg->pointer, 1);
	}
	// Add -1.
	for (i = 0; i < threadarg->niterations; i++) {
		threadarg->addfunc(threadarg->pointer, -1);
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int nthreads = 1, niterations = 1;
	int opt;
	while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case ADDTEST_THREADS: case ADDTEST_ITERATIONS: case ADDTEST_YIELD:
			{
				errno = 0;
				int temp = strtol(optarg, NULL, 10);
				if (temp >= 0 && errno == 0) {
					if (opt == ADDTEST_THREADS) {
						nthreads = temp;
					} else if (opt == ADDTEST_ITERATIONS) {
						niterations = temp;
					} else {
						opt_yield = (temp != 0);
					}
				}
				break;
			}
		}
	}

	long long counter = 0;

	struct timespec start_time;
	if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0) {
		fprintf(stderr, "Error getting start time.\n");
		return 1;
	}

	pthread_t *threads = malloc(sizeof(pthread_t) * nthreads);
	if (threads == NULL && nthreads > 0) {
		fprintf(stderr, "Error allocating thread array.\n");
		return 1;
	}
	threadarg_t *threadargs = malloc(sizeof(threadarg_t) * nthreads);
	if (threadargs == NULL && nthreads > 0) {
		fprintf(stderr, "Error allocating threadargs array.\n");
		free(threads);
		return 1;
	}

	int i;
	for (i = 0; i < nthreads; i++) {
		threadargs[i] = (threadarg_t) { niterations, &counter, add };
		if (pthread_create(&threads[i], NULL, process, &threadargs[i]) != 0) {
			if (errno == EAGAIN) {
				fprintf(stderr, "Too many threads? errno=EAGAIN\n");
			}
			fprintf(stderr, "Error creating thread %u; proceeding to next thread.\n", i);
			// Indicate that this thread failed to be created.
			threadargs[i].addfunc = NULL;
		}
	}
	for (i = 0; i < nthreads; i++) {
		// If the thread was successfully created.
		if (threadargs[i].addfunc != NULL) {
			if (pthread_join(threads[i], NULL) != 0) {
				fprintf(stderr, "Error waiting for thread %u; proceeding to next thread.\n", i);
			}
		}
	}

	free(threads);
	free(threadargs);

	struct timespec end_time;
	if (clock_gettime(CLOCK_MONOTONIC, &end_time) != 0) {
		fprintf(stderr, "Error getting end time.\n");
		return 1;
	}
	if (counter != 0) {
		fprintf(stderr, "ERROR: final count = %lld\n", counter);
	}

	int noperations = nthreads * niterations * 2;
	printf("%d threads x %d iterations x (add + subtract) = %d operations\n", nthreads, niterations, noperations);
	long totaltime = (end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
	printf("elapsed time: %ld ns\n", totaltime);
	printf("per operation: %ld ns\n", totaltime / noperations);
}
