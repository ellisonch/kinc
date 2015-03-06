.PHONY: run test force
.SUFFIXES:

TESTS = basic

run: test

# tools: kinc.exe libkinc-safe.a libaterm.a

libkinc-safe.a: force
	make -C ../imp libkinc-safe.a
	cp ../imp/libkinc-safe.a .

kinc.exe: force
	make -C ../src/ kinc.exe
	cp ../src/kinc.exe .

libaterm.a: force
	make -C ../imp/aparser
	cp ../imp/aparser/libaterm.a .

test: basic.test
#$(addsuffix .test, $(TESTS))
# echo hi

test.exe: test.go
	go build test.go

%.test: %.exe test.exe
	./test.exe $*

%.exe: %.c libkinc-safe.a libaterm.a
	@gcc -static -std=c11 -Wall -Wno-unused-variable -m64 -O3 $< -L. -lkinc-safe -laterm -DprintDebug=0 -I../imp -o $@

%.c: kinc.exe
	@./kinc.exe $*/$*.kinc > $*.c


# gcc -static -std=c99 -Wall -m64 -O3 lang.c -L. -lkinc-safe -laterm -DprintDebug=0 -I../imp