CC=gcc
CFLAGS=-std=c11 -O0 -g
build: imp.exe

run: imp.exe
	./imp.exe

imp.exe: Makefile imp.c k.h k.c utils.h utils.c settings.h
	$(CC) $(CFLAGS) imp.c utils.c k.c -o imp.exe
