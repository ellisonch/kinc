CC=gcc
CFLAGS=-static -std=c11 -Wall -g -gdwarf-2 -m64 -fno-omit-frame-pointer -O3 -static
# -flto
# -pedantic
build: imp.exe

# SRCS=$(wildcard *.c)
# OBJS=$(SRCS:.c=.o)

bench: imp.exe
	time ./imp.exe --bench

test: imp.exe
	time ./imp.exe --test

run: imp.exe
	time ./imp.exe --input 500000 programs/prog1.aterm

# %.o: %.c

imp.exe: Makefile imp.c k.h k.c utils.h utils.c settings.h cells.h cells.c k_labels.h k_labels.c k_builtins.h k_builtins.c adopt.h imp.h test.c test.h
	$(CC) $(CFLAGS) imp.c utils.c k.c cells.c k_labels.c k_builtins.c adopt.c test.c -L./aparser -laterm -o imp.exe

profile: imp.exe
	sleepy //r:"imp.exe --input 1000000 programs/prog1.aterm"