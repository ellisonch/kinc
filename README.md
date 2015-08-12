# kinc
This is an implementation of [K](http://www.kframework.org/) in C.
K is a framework for defining programming languages.
The idea is to formally define a programming language in K and then ```kinc``` will generate an interpreter for that language in C.
Using K to define a programming language is much easier than writing an interpreter directly (among other benefits).


## Details
```kinc``` is written in Go (though it generates C).
The interpreter generated basically simulates the rewrite rules that make up a K definition.
The resulting interpreter uses reference counting to deal with the frequent creation and deletion of terms.

The generated interpreter benchmarks at around 70x slower than a Python interpreter running on the same programs, though with some work 5x seems reachable (a hand-tweaked interpreter in the same style achieves such speeds).


## Caveats
* ```kinc``` is dramatically incomplete and unfinished
* In its current state, it can compile only a simple imperative language called IMP
* It's only been tested this on Cygwin on Windows
* The idea was scrapped in favor of a OCaml interpreter which is currently [being developed](https://github.com/kframework/k)
* Probably ```kinc``` is only useful as example code
