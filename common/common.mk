# Build rules shared by every topic directory.
#
# Each .c file in a directory is a separate program: foo.c builds ./foo.
# Pass extra flags with EXTRA_CFLAGS, e.g.  make EXTRA_CFLAGS=-Werror

CC      = gcc
CFLAGS  = -std=c11 -D_DEFAULT_SOURCE -Wall -Wextra -g -I../common \
          $(EXTRA_CFLAGS)
LDLIBS  = -pthread -lrt

PROGS   = $(basename $(wildcard *.c))

all: $(PROGS)

$(PROGS): %: %.c $(wildcard *.h) ../common/errexit.h
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f $(PROGS) $(CLEAN_EXTRA)

.PHONY: all clean
