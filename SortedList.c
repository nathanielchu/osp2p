#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "SortedList.h"

#define start_critical_section() \
if (sync == SYNC_MUTEX) { \
	if (pthread_mutex_lock(&slmutex) != 0) { \
		fprintf(stderr, "Error locking mutex.\n"); \
	} \
} else if (sync == SYNC_SPINLOCK) { \
	while(__sync_lock_test_and_set(&sllock, 1)) while (sllock); \
}

#define end_critical_section() \
if (sync == SYNC_MUTEX) { \
	pthread_mutex_unlock(&slmutex); \
} else if (sync == SYNC_SPINLOCK) { \
	__sync_lock_release(&sllock); \
}

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	if (list == NULL) {
		return;
	}

	// Needs to start here in case a list element is deleted while we're searching through it.
	start_critical_section();
	while (list->next != NULL && strcmp(element->key, list->next->key) >= 0) {
		list = list->next;
	}
	element->prev = list;
	element->next = list->next;
	if (list->next != NULL) {
		list->next->prev = element;
	}
	if (opt_yield & INSERT_YIELD) {
		pthread_yield();
	}
	list->next = element;
	end_critical_section();
}

int SortedList_delete(SortedListElement_t *element) {
	// Needs to start here in case an adjacent list element is deleted while we're deleting.
	start_critical_section();
	int hasnext = element->next != NULL;
	int hasprev = element->prev != NULL;
	if ((!hasnext || element->next->prev == element)
		&& (!hasprev || element->prev->next == element)) {
		if (hasnext) {
			element->next->prev = element->prev;
		}
		if (opt_yield & DELETE_YIELD) {
			pthread_yield();
		}
		if (hasprev) {
			element->prev->next = element->next;
		}
		end_critical_section();
		return 0;
	}
	end_critical_section();
	return 1;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	if (list == NULL) {
		return NULL;
	}

	// Needs to start here in case a list element is deleted while we're searching.
	start_critical_section();
	list = list->next;
	while (list != NULL && strcmp(list->key, key) < 0) {
		list = list->next;
	}
	if (opt_yield & SEARCH_YIELD) {
		pthread_yield();
	}
	if (strcmp(key, list->key) == 0) {
		end_critical_section();
		return list;
	}
	end_critical_section();
	return NULL;
}

int SortedList_length(SortedList_t *list) {
	// After first dummy node, will be 0.
	int length = -1;
	// Needs to start here in case a list element is deleted.
	start_critical_section();
	while (list != NULL) {
		if ((list->next == NULL || list->next->prev == list)
			&& (list->prev == NULL || list->prev->next == list)) {
			length++;
			if (opt_yield & SEARCH_YIELD) {
				pthread_yield();
			}
			list = list->next;
		} else {
			end_critical_section();
			return -1;
		}
	}
	end_critical_section();
	return length;
}
