# Build every topic directory:   make
# Treat warnings as errors:      make EXTRA_CFLAGS=-Werror
# Remove binaries and demo files: make clean

TOPICS = $(wildcard [0-9][0-9]-*)

all clean:
	@for d in $(TOPICS); do $(MAKE) -C $$d $@ || exit 1; done

.PHONY: all clean
