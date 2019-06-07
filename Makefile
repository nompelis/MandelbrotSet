CC = gcc
COPTS = -Wall -O2 -fPIC 


all:
	$(CC) -c $(COPTS) main.c
	$(CC) main.o -lm

clean:
	rm -f *.o a.out

