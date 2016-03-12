CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -lpthread -lrt

default: all

all: addtest sltest

addtest: addtest.c addtest.h
	$(CC) $(CFLAGS) addtest.c -o $@

sltest: sltest.c sltest.h SortedList.c SortedList.h
	$(CC) $(CFLAGS) sltest.c SortedList.c -o $@

.PHONY: default all clean

clean:
	rm -f *.tar.gz *.o addtest sltest
