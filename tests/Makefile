.PHONY: run test force
.SUFFIXES:

TESTS = basic imp

run: test

# tools: kinc.exe libkinc-safe.a libaterm.a

libkinc-safe.a: force
	make -C ../src/runtime libkinc-safe.a
	cp ../src/runtime/libkinc-safe.a .

kinc.exe: force
	make -C ../src/compiler kinc.exe
	cp ../src/compiler/kinc.exe .

libaterm.a: force
	make -C ../src/aterm-c
	cp ../src/aterm-c/libaterm.a .

test: basic.test imp.test fun.test
#$(addsuffix .test, $(TESTS))
# echo hi

test.exe: test.go
	go build test.go

%.test: %.exe test.exe
	- ./test.exe $*

%.exe: %.c libkinc-safe.a libaterm.a
	@gcc -static -std=c11 -Wall -Wno-unused-variable -m64 -O3 $< -L. -lkinc-safe -laterm -DprintDebug=0 -I../src/runtime -o $@

%.c: kinc.exe
	@./kinc.exe $*/$*.kinc > $*.c


# gcc -static -std=c99 -Wall -m64 -O3 lang.c -L. -lkinc-safe -laterm -DprintDebug=0 -I../imp