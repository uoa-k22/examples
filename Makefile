# Build every topic directory:   make
# Treat warnings as errors:      make EXTRA_CFLAGS=-Werror
# Remove binaries and demo files: make clean
# Check code and docs:           make lint  (needs cppcheck, markdownlint-cli2)

TOPICS = $(wildcard [0-9][0-9]-*)

all clean:
	@for d in $(TOPICS); do $(MAKE) -C $$d $@ || exit 1; done

# Lines of at most 80 columns, then the C linter, then the Markdown linter.
lint:
	./tools/check-columns.sh
	cppcheck --enable=warning,style,performance,portability --std=c11 \
	    -D_DEFAULT_SOURCE -Icommon --inline-suppr \
	    --suppressions-list=.cppcheck-suppressions \
	    --error-exitcode=1 --quiet $(TOPICS) common
	markdownlint-cli2 "**/*.md"

.PHONY: all clean lint
