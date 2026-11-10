include ../common/common.mk

# Writer and reader, one after the other.
run: 06_writer 07_reader
	./06_writer "hello from the writer"
	ipcs -m
	./07_reader

.PHONY: run
