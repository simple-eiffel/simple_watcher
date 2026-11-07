# POSIX build of the object simple_watcher.ecf expects on macOS and Linux.
# Usage: make -C Clib
CC ?= cc
CFLAGS ?= -O2 -fPIC

simple_watcher.o: simple_watcher.c simple_watcher.h
	$(CC) $(CFLAGS) -c simple_watcher.c -o simple_watcher.o

clean:
	rm -f simple_watcher.o

.PHONY: clean
