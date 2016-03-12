#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "SortedList.h"

extern int optyield;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	if (list == NULL) {
		return;
	}

	while (list->next != NULL && strcmp(element->key, list->next->key) >= 0) {
		list = list->next;
	}
	element->prev = list;
	element->next = list->next;
	if (list->next != NULL) {
		list->next->prev = element;
	}
	if (optyield & INSERT_YIELD) {
		pthread_yield();
	}
	list->next = element;
}

int SortedList_delete(SortedListElement_t *element) {
	int hasnext = element->next != NULL;
	int hasprev = element->prev != NULL;
	if ((!hasnext || element->next->prev == element)
		&& (!hasprev || element->prev->next == element)) {
		if (hasnext) {
			element->next->prev = element->prev;
		}
		if (optyield & DELETE_YIELD) {
			pthread_yield();
		}
		if (hasprev) {
			element->prev->next = element->next;
		}
		return 0;
	}
	return 1;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	if (list == NULL) {
		return NULL;
	}

	list = list->next;
	while (list != NULL && strcmp(key, list->key) < 0) {
		list = list->next;
	}
	if (optyield & SEARCH_YIELD) {
		pthread_yield();
	}
	if (strcmp(key, list->key) == 0) {
		return list;
	}
	return NULL;
}

int SortedList_length(SortedList_t *list) {
	// After first dummy node, will be 0.
	int length = -1;
	while (list != NULL) {
		if ((list->next == NULL || list->next->prev == list)
			&& (list->prev == NULL || list->prev->next == list)) {
			length++;
			if (optyield & SEARCH_YIELD) {
				pthread_yield();
			}
			list = list->next;
		} else {
			return -1;
		}
	}
	return length;
}
