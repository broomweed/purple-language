CC=gcc
CFLAGS=-std=c99 -Wall -g

purple: purple.c stack.o stack.h
	$(CC) $(CFLAGS) purple.c stack.o stack.h -o purple


stack.o: stack.c
	$(CC) $(CFLAGS) -c stack.c
