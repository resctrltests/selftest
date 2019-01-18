CC = gcc
CFLAGS = -g -Wall

all: resctrl_tests

*.o: *.c
	$(CC) $(CFLAGS) -c *.c

resctrl_tests: *.o
	$(CC) $(CFLAGS) -o resctrl_tests resctrl_tests.o resctrlfs.o \
		 resctrl_val.o fill_buf.o mbm_test.o mba_test.o cache.o cqm_test.o

.PHONY: clean

clean:
	$(RM) *.o *~ resctrl_tests
