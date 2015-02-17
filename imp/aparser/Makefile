all: atermparser.exe

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

atermparser.exe: lex.yy.c parser.tab.c aterm.c aterm.h
	gcc parser.tab.c lex.yy.c aterm.c -o atermparser.exe