.PHONY: all debug safe fast
.SUFFIXES:

DIST = ../../dist/
CC = gcc
CFLAGS = -static -std=c11 -Wall -m64 -I../aterm-c/

DEBUG_FLAGS = -O0 -DprintRefCounts=1 -DprintDebug=1
SAFE_FLAGS = -O3 -DprintRefCounts=0 -DprintDebug=0
FAST_FLAGS = -O3 -DprintRefCounts=0 -DprintDebug=0 -flto -DNDEBUG -Wno-unused-variable

HEADERS = $(wildcard ./*.h)
FILES = $(wildcard ./*.c)
# LIBS = -L./aparser -laterm
# -pedantic

# gdb --args ./imp.exe --test

all: debug safe fast

dist: debug safe fast
	mkdir -p $(DIST)
	cp ./libkinc-debug.a $(DIST)
	cp ./libkinc-safe.a $(DIST)
	cp ./libkinc-fast.a $(DIST)

debug: libkinc-debug.a
safe: libkinc-safe.a
fast: libkinc-fast.a

%-debug.o: $(HEADERS) Makefile
	$(CC) -c $(CFLAGS) $(DEBUG_FLAGS) $*.c -o $@

%-safe.o: $(HEADERS) Makefile
	$(CC) -c $(CFLAGS) $(SAFE_FLAGS) $*.c -o $@

%-fast.o: $(HEADERS) Makefile
	$(CC) -c $(CFLAGS) $(FAST_FLAGS) $*.c -o $@

libkinc-%.a: ${FILES:.c=-%.o}
	ar rcs $@ $^
