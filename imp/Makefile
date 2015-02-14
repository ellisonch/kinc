CC=gcc
CFLAGS=-std=c11 -O3 -Wall -g -gdwarf-2 -fno-omit-frame-pointer
# -pedantic
build: imp.exe

# SRCS=$(wildcard *.c)
# OBJS=$(SRCS:.c=.o)

run: imp.exe
	time ./imp.exe

run-big: imp.exe
	time ./imp.exe 5000000

# %.o: %.c

imp.exe: Makefile imp.c k.h k.c utils.h utils.c settings.h cells.h cells.c k_labels.h k_labels.c k_builtins.h k_builtins.c
	$(CC) $(CFLAGS) imp.c utils.c k.c cells.c k_labels.c k_builtins.c -o imp.exe

profile: imp.exe
	cmd /C "sleepy /r:imp.exe 2000000"