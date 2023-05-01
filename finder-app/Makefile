CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall -g


default: 
	$(CC) writer.c $(CFLAGS) -o writer

.PHONY: clean

clean:
	rm -f  writer writer.o