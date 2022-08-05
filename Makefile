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

all: libc-stache.a test

clean:
	rm -f $(C_STACHE_OBJ) libc-stache.a test.o test

check: test
	./test

libc-stache.a: $(C_STACHE_OBJ)
	$(AR) $(ARFLAGS) $@ $(C_STACHE_OBJ)

test: test.o libc-stache.a
	$(LD) $(LDFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

c-stache.o test.o: c-stache.h
test.o: dh_cuts.h

