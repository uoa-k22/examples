# Tests that the example programs do what their header comments say.
# There is one test program per topic, built with Acutest (acutest.h).
#
#   make test            build and run them all (from the top directory)
#   ./01-fork --help     Acutest's options, e.g. run only some tests
#
# Each one runs inside its topic directory, so build the topics first.

CC      = gcc
CFLAGS  = -std=c11 -D_DEFAULT_SOURCE -Wall -Wextra -g $(EXTRA_CFLAGS)

TESTS   = $(basename $(wildcard [0-9][0-9]-*.c))

all: $(TESTS)

$(TESTS): %: %.c run.h acutest.h
	$(CC) $(CFLAGS) $< -o $@

# Run every topic's tests, even after a failure, and fail at the end.
test: $(TESTS)
	@status=0; \
	for t in $(TESTS); do \
	    echo "=== $$t"; \
	    (cd ../$$t && ../tests/$$t) || status=1; \
	done; \
	exit $$status

clean:
	rm -f $(TESTS)

.PHONY: all test clean
