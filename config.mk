# Customize below to fit your system

# compiler and linker
CC = gcc
LD = gcc
AR = ar

# installation paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

# compiler flags for c-stache
CPPFLAGS =
CFLAGS   = -g -Wall
LDFLAGS  = -g
ARFLAGS  = rcs

# compiler flags for the test application
TEST_CPPFLAGS =
TEST_CFLAGS   = -g -Wall
TEST_LDFLAGS  = -g

