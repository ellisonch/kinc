CC=gcc

build: imp.exe


run: imp.exe
	./imp.exe



imp.exe: Makefile imp.c k.h utils.h utils.c
	$(CC) -O3 -g -std=c11 imp.c utils.c -o imp.exe