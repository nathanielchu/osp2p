CC = gcc
CFLAGS = -std=c11 -Wall -Wextra

default: all

all: addtest

addtest: addtest.c addtest.h
	$(CC) $(CFLAGS) addtest.c -o $@

.PHONY: default all clean

clean:
	rm -f *.tar.gz *.o addtest
