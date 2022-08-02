CC=gcc
LD=gcc
AR=ar
CFLAGS=-g -Wall
CPPFLAGS=
ARFLAGS=rcs
LDFLAGS=-g

C_STACHE_OBJ=c-stache.o

.POSIX:

.PHONY: all clean check

all: libc-stache.a tests

clean:
	rm -f $(C_STACHE_OBJ) libc-stache.a

libc-stache.a: $(C_STACHE_OBJ)
	$(AR) $(ARFLAGS) $@ $(C_STACHE_OBJ)

tests: tests.o libc-stache.a
	$(LD) $(LDFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

c-stache.o tests.o: c-stache.h
tests.o: dh_cuts.h

