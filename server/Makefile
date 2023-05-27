CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall -g


aesdsocket: 
	$(CC) *.c $(CFLAGS) -o aesdsocket 

all: aesdsocket

default: aesdsocket

.PHONY: clean

clean:
	rm -f  aesdsocket  *.o