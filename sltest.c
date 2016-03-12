#define _GNU_SOURCE
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sltest.h"
#include "SortedList.h"

// Return NULL if success, anything else otherwise.
void *process(void *arg) {
	SortedListElement_t *elements = arg;
	int status = 0;
	int i;
	// Insert each element.
	for (i = 0; i < niterations; i++) {
		SortedList_insert(&list, &elements[i]);
	}
	// Determine length.
	if (SortedList_length(&list) == -1) {
		status = 1;
	}
	// Lookup each element.
	for (i = 0; i < niterations; i++) {
		SortedListElement_t *element = SortedList_lookup(&list, elements[i].key);
		// Delete each found element.
		if (element != NULL && SortedList_delete(element) != 0) {
			status = 1;
		}
	}
	return (status == 0) ? NULL : arg;
}

int main(int argc, char *argv[]) {
	int opt;
	while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case SLTEST_THREADS: case SLTEST_ITERATIONS:
			{
				errno = 0;
				int temp = strtol(optarg, NULL, 10);
				if (temp >= 0 && errno == 0) {
					if (opt == SLTEST_THREADS) {
						nthreads = temp;
					} else {
						niterations = temp;
					}
				}
				break;
			}
			case SLTEST_YIELD:
			{
				int i;
				for (i = 0; optarg[i] != '\0'; i++) {
					switch (optarg[i]) {
						case 'i': opt_yield |= INSERT_YIELD; break;
						case 'd': opt_yield |= DELETE_YIELD; break;
						case 's': opt_yield |= SEARCH_YIELD; break;
					}
				}
				break;
			}
			case SLTEST_SYNC:
			{
				if (optarg[0] == 'm') {
					sync = SYNC_MUTEX;
					if (pthread_mutex_init(&slmutex, NULL) != 0) {
						fprintf(stderr, "Error initializing mutex.\n");
						return 1;
					}
				} else if (optarg[0] == 's') {
					sync = SYNC_SPINLOCK;
				}
				break;
			}
		}
	}

	list = (SortedList_t) { NULL, NULL, NULL };
	SortedListElement_t *elements = malloc(sizeof(SortedListElement_t) * nthreads * niterations);
	if (elements == NULL && nthreads > 0 && niterations > 0) {
		fprintf(stderr, "Error allocating elements array.\n");
		return 1;
	}
	char *keys = malloc(sizeof(char) * (KEY_MAX_LENGTH+1) * nthreads * niterations);
	if (keys == NULL && nthreads > 0 && niterations > 0) {
		fprintf(stderr, "Error allocating keys array.\n");
		free(elements);
		return 1;
	}
	srandom(time(NULL));
	int i;
	for (i = 0; i < nthreads * niterations; i++) {
		char *key = &keys[i * (KEY_MAX_LENGTH+1)];
		int keylength = 1 + (random() % KEY_MAX_LENGTH);
		int j;
		for (j = 0; j < keylength; j++) {
			key[j] = 'A' + (random() % 26);
		}
		key[keylength] = '\0';
		elements[i] = (SortedListElement_t) { NULL, NULL, key };
	}

	struct timespec start_time;
	if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0) {
		fprintf(stderr, "Error getting start time.\n");
		free(keys); free(elements);
		return 1;
	}
	
	pthread_t *threads = malloc(sizeof(pthread_t) * nthreads);
	if (threads == NULL && nthreads > 0) {
		fprintf(stderr, "Error allocating thread array.\n");
		free(keys); free(elements);
		return 1;
	}
	// Keep track of return status from pthread_create in case it fails.
	int *threadstatus = malloc(sizeof(int) * nthreads);
	if (threadstatus == NULL && nthreads > 0) {
		fprintf(stderr, "Error allocating threadstatus array.\n");
		free(threads);
		free(keys); free(elements);
		return 1;
	}

	int status = 0;
	for (i = 0; i < nthreads; i++) {
		threadstatus[i] = pthread_create(&threads[i], NULL, process, &elements[i * niterations]);
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
		void *exitstatus = NULL;
		if (threadstatus[i] == 0 && pthread_join(threads[i], &exitstatus) != 0) {
			fprintf(stderr, "Error waiting for thread %d; proceeding to next thread.\n", i);
			status = 1;
		} else if (exitstatus != NULL) {
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
	int listlength = SortedList_length(&list);
	if (listlength != 0) {
		fprintf(stderr, "ERROR: final length = %d\n", listlength);
		status = 1;
	}
	free(keys);
	free(elements);

	if (sync == SYNC_MUTEX && pthread_mutex_destroy(&slmutex) != 0) {
		fprintf(stderr, "Error destroying mutex.\n");
		status = 1;
	}

	int noperations = nthreads * niterations * niterations;
	printf("%d threads x %d iterations x (ins + lookup/del) x (%d/2 avg len) = %d operations\n", nthreads, niterations, niterations, noperations);
	long totaltime = (end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
	printf("elapsed time: %ld ns\n", totaltime);
	printf("per operation: %ld ns\n", totaltime / noperations);

	return status;
}
