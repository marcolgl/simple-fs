CCOPTS = -Wall -g -std=gnu99 -Wstrict-prototypes
CC = gcc

BINS = bit-main

HEADERS = bitmap.h

.phony: clean all

all:	$(BINS)

bit-main:	bit-main.c bitmap.c $(HEADERS)
	$(CC) $(CCOPTS) -o $@ $^ 



clean:
	rm -rf *.o *~  $(BINS)
