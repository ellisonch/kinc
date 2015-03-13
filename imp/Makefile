CC=gcc
CFLAGS=-static -std=c11 -Wall -m64 -O3
RELEASE_FLAGS=-DprintRefCounts=1 -DprintDebug=1
TEST_FLAGS=-DprintRefCounts=1 -DprintDebug=1

FILES = imp_bench.c utils.c k.c cells.c k_labels.c k_builtins.c adopt.c test.c
LIBS = -L./aparser -laterm
DEPENDS = $(FILES) imp_bench.h k.h k_types.h utils.h settings.h cells.h k_labels.h k_builtins.h adopt.h lang.h test.h k_pub.h

NEWFILES = lang_helper.c utils.c k.c cells.c k_labels.c k_builtins.c adopt.c
NEWHEADERS = lang.h k.h k_types.h utils.h settings.h cells.h k_labels.h k_builtins.h adopt.h lang.h k_pub.h
NEWLIBS = -L./aparser -laterm
NEWDEPENDS = $(NEWFILES) $(NEWHEADERS)

# -DNDEBUG
# -Wno-unused-function
# -flto
# -pedantic

# gdb --args ./imp.exe --test

all: simp



# parser.tab.o lex.yy.o aterm.o: lex.yy.c parser.tab.c aterm.c aterm.h
# 	gcc -c parser.tab.c lex.yy.c aterm.c

*.o: $(NEWHEADERS) Makefile

libkinc-safe.a: ${NEWFILES:.c=.o}
	ar rcs libkinc-safe.a ${NEWFILES:.c=.o}

# lang: lang.exe

lang.exe: Makefile $(NEWDEPENDS) lang.c
	echo making lang.exe
	$(CC) $(CFLAGS) $(TEST_FLAGS) -O0 $(NEWFILES) $(NEWLIBS) lang.c -o lang.exe
	echo made lang.exe

# lang.c: ../testlang/testlang.kinc kinc
#	../src/kinc.exe ../testlang/testlang.kinc > lang.c

lang.c: ../tests/imp/imp.kinc kinc
	../src/kinc.exe ../tests/imp/imp.kinc > lang.c

kinc:
	make -C ../src kinc.exe

simp: imp-simp.exe
	./imp-simp.exe programs/prog1.aterm

imp-simp.exe: Makefile $(NEWDEPENDS)
	$(CC) $(CFLAGS) -O0 $(NEWFILES) $(NEWLIBS) imp.c -o imp-simp.exe

build: imp.exe

# SRCS=$(wildcard *.c)
# OBJS=$(SRCS:.c=.o)

bench: imp.exe
	time ./imp.exe --bench

bench-fast: imp-fast.exe
	time ./imp-fast.exe --bench	

test: imp.exe
	time ./imp.exe --test

mem: imp.exe
	time ./imp.exe --mem

run: imp.exe
	time ./imp.exe --input 500000 programs/prog1.aterm

imp.exe: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -DprintRefCounts=1 -g -flto $(FILES) $(LIBS) -o imp.exe

asm: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -S -g $(FILES)

imp-O1.exe: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -DNDEBUG -O1 -pg $(FILES) $(LIBS) -o imp-O1.exe

imp-fast.exe: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -DNDEBUG -flto $(FILES) $(LIBS) -o imp-fast.exe

imp-profile.exe: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -DNDEBUG -flto -pg $(FILES) $(LIBS) -o imp-profile.exe	

imp-sleepy.exe: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -g -fno-omit-frame-pointer -gdwarf-2 $(FILES) $(LIBS) -o imp-sleepy.exe	

imp-profile-O1.output: imp-O1.exe	
	./imp-O1.exe --bench
	gprof ./imp-O1.exe > imp-profile-O1.output

imp-profile.output: imp-profile.exe	
	./imp-profile.exe --bench
	gprof ./imp-profile.exe > imp-profile.output

profile: imp-profile.output
	less imp-profile.output

profile-O1: imp-profile-O1.output
	less imp-profile-O1.output

profile-sleepy: imp-sleepy.exe
	sleepy //r:"imp-sleepy.exe --bench" &

bench-python:
	time (python programs/prog1.py && python programs/prog2.py && python programs/prog3.py && python programs/prog4.py)