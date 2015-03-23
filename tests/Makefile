.PHONY: run test force
.SUFFIXES:

TESTS = basic imp

run: test

# tools: kinc.exe libkinc-safe.a libaterm.a

libkinc: force
#	make -C ../src/runtime libkinc-safe.a
#	cp ../src/runtime/libkinc-safe.a .
	make -C ../src/runtime dist

kinc: force
#	make -C ../src/compiler kinc
#	cp ../src/compiler/kinc .
	make -C ../src/compiler dist

libaterm: force
#	make -C ../src/aterm-c
#	cp ../src/aterm-c/libaterm.a .
	make -C ../src/aterm-c dist

test: basic.test imp.test fun.test
#$(addsuffix .test, $(TESTS))
# echo hi

runtest: test.go
	go build -o runtest test.go

%.test: %.exe runtest
	- ./test.exe $*

%.exe: %.c libkinc-safe.a libaterm.a
#	@gcc -static -std=c11 -Wall -Wno-unused-variable -m64 -O3 $< -L. -lkinc-safe -laterm -DprintDebug=0 -I../src/runtime -o $@
	kinclink-safe $< -o $@

%.c: kinc 
# %/%.kinc # can't use stem twice :/
	@./kinc $*/$*.kinc > $*.c


# gcc -static -std=c99 -Wall -m64 -O3 lang.c -L. -lkinc-safe -laterm -DprintDebug=0 -I../imp