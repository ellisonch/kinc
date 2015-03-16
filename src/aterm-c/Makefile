all: atermparser.exe

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

parser.tab.o lex.yy.o aterm.o: lex.yy.c parser.tab.c aterm.c aterm.h
	gcc -c parser.tab.c lex.yy.c aterm.c

libaterm.a: parser.tab.o lex.yy.o aterm.o
	ar rcs libaterm.a parser.tab.o lex.yy.o aterm.o

atermparser.exe: libaterm.a atermtest.c
	gcc -static atermtest.c -L. -laterm -o atermparser.exe