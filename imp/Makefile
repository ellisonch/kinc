CC=gcc
CFLAGS=-std=c11 -O3 -g
build: imp.exe

run: imp.exe
	time ./imp.exe

run-big: imp.exe
	time ./imp.exe 5000000

imp.exe: Makefile imp.c k.h k.c utils.h utils.c settings.h cells.h cells.c k_labels.h k_labels.c
	$(CC) $(CFLAGS) imp.c utils.c k.c cells.c k_labels.c -o imp.exe
