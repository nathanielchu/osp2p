#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "addtest.h"

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	*pointer = sum;
}

int main(int argc, char *argv[]) {
	int threads = 1, iterations = 1;
	int opt;
	while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case ADDTEST_THREADS: case ADDTEST_ITERATIONS:
			{
				int temp;
				errno = 0;
				temp = strtol(optarg, NULL, 10);
				if (temp != 0 || errno == 0) {
					if (opt == ADDTEST_THREADS) {
						threads = temp;
					} else {
						iterations = temp;
					}
				}
				break;
			}
		}
	}

	long long counter = 0;
}
