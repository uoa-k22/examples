include ../common/common.mk

# Writer and reader, one after the other.
run: 01_posix_shm_writer 02_posix_shm_reader
	./01_posix_shm_writer "hello from the writer"
	ls -l /dev/shm/k22_shm_demo
	./02_posix_shm_reader

.PHONY: run
