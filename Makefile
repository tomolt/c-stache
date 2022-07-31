CC=gcc
AR=ar
CFLAGS=-g
CPPFLAGS=
ARFLAGS=rcs

C_STACHE_OBJ=c-stache.o

.POSIX:

.PHONY: all clean

all: libc-stache.a

clean:
	rm -f $(C_STACHE_OBJ) libc-stache.a

libc-stache.a: $(C_STACHE_OBJ)
	$(AR) $(ARFLAGS) $@ $(C_STACHE_OBJ)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

c-stache.o: c-stache.h

