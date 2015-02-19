CC=gcc
CFLAGS=-static -std=c11 -Wall -m64 -O3
FILES = imp.c utils.c k.c cells.c k_labels.c k_builtins.c adopt.c test.c -L./aparser -laterm 
DEPENDS = imp.c k.h k.c k_types.h utils.h utils.c settings.h cells.h cells.c k_labels.h k_labels.c k_builtins.h k_builtins.c adopt.h imp.h test.c test.h inline_helper.h
# -DNDEBUG
# -Wno-unused-function
# -flto
# -pedantic

# gdb --args ./imp.exe --test

build: imp.exe

# SRCS=$(wildcard *.c)
# OBJS=$(SRCS:.c=.o)

bench: imp.exe
	time ./imp.exe --bench

bench-fast: imp-fast.exe
	time ./imp-fast.exe --bench	

test: imp.exe
	time ./imp.exe --test

run: imp.exe
	time ./imp.exe --input 500000 programs/prog1.aterm

imp.exe: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -g -flto $(FILES) -o imp.exe

asm: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -S -g $(FILES)

imp-fast.exe: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -DNDEBUG -flto $(FILES) -o imp-fast.exe

imp-profile.exe: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -DNDEBUG -flto -pg $(FILES) -o imp-profile.exe	

imp-sleepy.exe: Makefile $(DEPENDS)
	$(CC) $(CFLAGS) -g -fno-omit-frame-pointer -gdwarf-2 $(FILES) -o imp-sleepy.exe	

imp-profile.output: imp-profile.exe	
	./imp-profile.exe --bench
	gprof ./imp-profile.exe > imp-profile.output

profile: imp-profile.output
	less imp-profile.output

profile-sleepy: imp-sleepy.exe
	sleepy //r:"imp-sleepy.exe --bench" &

bench-python:
	time (python programs/prog1.py && python programs/prog2.py && python programs/prog3.py && python programs/prog4.py)