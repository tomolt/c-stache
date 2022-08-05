include config.mk

C_STACHE_OBJ=c-stache.o

.POSIX:

.PHONY: all clean check install uninstall

all: libc-stache.a test

clean:
	rm -f $(C_STACHE_OBJ) libc-stache.a test.o test

check: test
	./test

install: libc-stache.a c-stache.h
	# libc-stache.a
	mkdir -p "$(DESTDIR)$(PREFIX)/lib"
	cp -f libc-stache.a "$(DESTDIR)$(PREFIX)/lib"
	chmod 644 "$(DESTDIR)$(PREFIX)/lib/libc-stache.a"
	# c-stache.h
	mkdir -p "$(DESTDIR)$(PREFIX)/include"
	cp -f c-stache.h "$(DESTDIR)$(PREFIX)/include"
	chmod 644 "$(DESTDIR)$(PREFIX)/include/c-stache.h"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/lib/libc-stache.a"
	rm -f "$(DESTDIR)$(PREFIX)/include/c-stache.h"

libc-stache.a: $(C_STACHE_OBJ)
	$(AR) $(ARFLAGS) $@ $(C_STACHE_OBJ)

test: test.o libc-stache.a
	$(LD) $(LDFLAGS) -o $@ $^

c-stache.o: c-stache.c c-stache.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

test.o: test.c dh_cuts.h c-stache.h
	$(CC) $(TEST_CFLAGS) $(TEST_CPPFLAGS) -c -o $@ $<

