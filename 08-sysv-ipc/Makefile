include ../common/common.mk

# Writer and reader, one after the other.
run: 06_ftok_writer 07_ftok_reader
	./06_ftok_writer "hello from the writer"
	ipcs -m
	./07_ftok_reader

.PHONY: run
