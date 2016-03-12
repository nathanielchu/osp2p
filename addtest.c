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

void add_mutex(long long *pointer, long long value) {
	if (pthread_mutex_lock(&mutex) != 0) {
		fprintf(stderr, "Error locking mutex.\n");
		return;
	}

	long long sum = *pointer + value;
	if (opt_yield)
		pthread_yield();
	*pointer = sum;

	pthread_mutex_unlock(&mutex);
}

void add_spinlock(long long *pointer, long long value) {
	while(__sync_lock_test_and_set(&lock, 1)) while (lock);

	long long sum = *pointer + value;
	if (opt_yield)
		pthread_yield();
	*pointer = sum;

	__sync_lock_release(&lock);
}

void add_cas(long long *pointer, long long value) {
	long long sum;
	long long oldval;
	do {
		oldval = *pointer;
		sum = oldval + value;
		if (opt_yield)
			pthread_yield();
	} while (__sync_val_compare_and_swap(pointer, oldval, sum) != oldval);;
}

void *process(void *arg) {
	long long *pointer = arg;
	int i;
	// Add 1.
	for (i = 0; i < niterations; i++) {
		addfunc(pointer, 1);
	}
	// Add -1.
	for (i = 0; i < niterations; i++) {
		addfunc(pointer, -1);
	}
	return 0;
}

int main(int argc, char *argv[]) {
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
			case ADDTEST_SYNC:
			{
				switch(optarg[0]) {
					case 'm':
						addfunc = add_mutex;
						if (pthread_mutex_init(&mutex, NULL) != 0) {
							fprintf(stderr, "Error initializing mutex.\n");
							return 1;
						}
						break;
					case 's': addfunc = add_spinlock; break;
					case 'c': addfunc = add_cas; break;
				}
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
	// Keep track of return status from pthread_create in case it fails.
	int *threadstatus = malloc(sizeof(int) * nthreads);
	if (threadstatus == NULL && nthreads > 0) {
		fprintf(stderr, "Error allocating threadstatus array.\n");
		free(threads);
		return 1;
	}

	int status = 0;
	int i;
	for (i = 0; i < nthreads; i++) {
		threadstatus[i] = pthread_create(&threads[i], NULL, process, &counter);
		if (threadstatus[i] != 0) {
			if (errno == EAGAIN) {
				fprintf(stderr, "Too many threads? errno=EAGAIN\n");
			}
			fprintf(stderr, "Error creating thread %d; proceeding to next thread.\n", i);
			status = 1;
		}
	}
	for (i = 0; i < nthreads; i++) {
		// Make sure thread was successfully created.
		if (threadstatus[i] == 0 && pthread_join(threads[i], NULL) != 0) {
			fprintf(stderr, "Error waiting for thread %d; proceeding to next thread.\n", i);
			status = 1;
		}
	}

	free(threads);
	free(threadstatus);

	struct timespec end_time;
	if (clock_gettime(CLOCK_MONOTONIC, &end_time) != 0) {
		fprintf(stderr, "Error getting end time.\n");
		return 1;
	}
	if (counter != 0) {
		fprintf(stderr, "ERROR: final count = %lld\n", counter);
		status = 1;
	}

	if (addfunc == add_mutex && pthread_mutex_destroy(&mutex) != 0) {
		fprintf(stderr, "Error destroying mutex.\n");
		status = 1;
	}

	int noperations = nthreads * niterations * 2;
	printf("%d threads x %d iterations x (add + subtract) = %d operations\n", nthreads, niterations, noperations);
	long totaltime = (end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
	printf("elapsed time: %ld ns\n", totaltime);
	printf("per operation: %ld ns\n", totaltime / noperations);

	return status;
}
