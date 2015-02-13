CC=gcc
CFLAGS=-std=c11 -O3 -g
build: imp.exe

run: imp.exe
	time ./imp.exe

imp.exe: Makefile imp.c k.h k.c utils.h utils.c settings.h cells.h cells.c
	$(CC) $(CFLAGS) imp.c utils.c k.c cells.c -o imp.exe
