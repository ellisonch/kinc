.PHONY: all debug safe fast
.SUFFIXES:
.SECONDARY:

DIST = ../../dist
CC = gcc
CFLAGS = -static -std=c11 -Wall -m64
CFLAGS_LOCAL = -I$(DIST)/include

DIST_DIR := $(shell readlink -f $(DIST))
# INCLUDE_DIR := $(shell readlink -f .)

DEBUG_FLAGS = -O0 -DprintRefCounts=1 -DprintDebug=1
SAFE_FLAGS = -O3 -DprintRefCounts=0 -DprintDebug=0
FAST_FLAGS = -O3 -DprintRefCounts=0 -DprintDebug=0 -flto -DNDEBUG -Wno-unused-variable
PGPROFILE_FLAGS = -O3 -DprintRefCounts=0 -DprintDebug=0 -flto -DNDEBUG -pg -Wno-unused-variable

HEADERS = $(wildcard ./*.h)
FILES = $(wildcard ./*.c)
# LIBS = -L./aparser -laterm
# -pedantic

# gdb --args ./imp.exe --test

all: debug safe fast

prepareDist:
	rm -rf $(DIST)
	mkdir -p $(DIST)/include
	mkdir -p $(DIST)/lib
	mkdir -p $(DIST)/bin

aterm: prepareDist
	make -C ../aterm-c dist

dist: aterm dist-debug dist-safe dist-fast dist-pgprofile
	cp $(HEADERS) $(DIST)/include
	
# debug safe fast	
	# cp ./libkinc-debug.o $(DIST)
	# cp ./libkinc-safe.o $(DIST)
	# cp ./libkinc-fast.o $(DIST)
	# cp ./libkinc-pgprofile.o $(DIST)

dist-%: kinclink-% prepareDist ofiles-%
	cp $< $(DIST)/bin

ofiles-%: ${FILES:.c=-%o.o}
	cp $^ $(DIST)/lib

kinclink-debug: Makefile
	printf "%s %s %s %s -Wno-unused-variable -laterm \"\$$@\"\n" "$(CC)" "$(CFLAGS)" "$(DEBUG_FLAGS)" "${FILES:%.c=$(DIST_DIR)/lib/%-debugo.o}" > $@
kinclink-safe: Makefile
	printf "%s %s %s %s -Wno-unused-variable -laterm \"\$$@\"\n" "$(CC)" "$(CFLAGS)" "$(SAFE_FLAGS)" "${FILES:%.c=$(DIST_DIR)/lib/%-safeo.o}" > $@
kinclink-fast: Makefile
	printf "%s %s %s %s -Wno-unused-variable -laterm \"\$$@\"\n" "$(CC)" "$(CFLAGS)" "$(FAST_FLAGS)" "${FILES:%.c=$(DIST_DIR)/lib/%-fasto.o}" > $@
kinclink-pgprofile: Makefile
	printf "%s %s %s %s -Wno-unused-variable -laterm \"\$$@\"\n" "$(CC)" "$(CFLAGS)" "$(PGPROFILE_FLAGS)" "${FILES:%.c=$(DIST_DIR)/lib/%-pgprofileo.o}" > $@


# debug: libkinc-debug.o
# safe: libkinc-safe.o
# fast: libkinc-fast.o
# pgprofileo: libkinc-pgprofile.o

%-debugo.o: aterm $(HEADERS) Makefile
	$(CC) -c $(CFLAGS) $(CFLAGS_LOCAL) $(DEBUG_FLAGS) $*.c -o $@

%-safeo.o: aterm $(HEADERS) Makefile
	$(CC) -c $(CFLAGS) $(CFLAGS_LOCAL) $(SAFE_FLAGS) $*.c -o $@

%-fasto.o: aterm $(HEADERS) Makefile
	$(CC) -c $(CFLAGS) $(CFLAGS_LOCAL) $(FAST_FLAGS) $*.c -o $@

%-pgprofileo.o: aterm $(HEADERS) Makefile
	$(CC) -c $(CFLAGS) $(CFLAGS_LOCAL) $(PGPROFILE_FLAGS) $*.c -o $@

# libkinc-%.a: ${FILES:.c=-%.o}
# 	rm -f $@
# 	ar rcs $@ $^

# libkinc-%.o: ${FILES:.c=-%o.o}
# 	ld -r -o $@ $^

