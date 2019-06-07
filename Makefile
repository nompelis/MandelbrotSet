CC = gcc
COPTS = -Wall -O0 -fPIC 


all:
	$(CC) -c $(COPTS) main.c
	$(CC) main.o

