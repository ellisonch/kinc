# kinc
This is an implementation of [K](http://www.kframework.org/) in C.
K is a framework for defining programming languages.
The idea is to formally define a programming language in K and then ```kinc``` will generate an interpreter for that language in C.
Using K to define a programming language is much easier than writing an interpreter directly (among other benefits).
```kinc``` served as a prototype for faster K backends.

## Details
```kinc``` is written in Go (though it generates C).
The interpreter generated basically simulates the rewrite rules that make up a K definition.
The resulting interpreter uses reference counting to deal with the frequent creation and deletion of terms.

The generated interpreter benchmarks at around 70x slower than a Python interpreter running on the same programs, though with some work 5x seems reachable (a hand-tweaked interpreter in the same style achieves such speeds).


## Caveats
* ```kinc``` is dramatically incomplete and unfinished
* In its current state, it can compile only a simple imperative language called IMP
* It's only been tested on Cygwin on Windows
* The idea was scrapped in favor of a OCaml interpreter which is currently [being developed](https://github.com/kframework/k)
* Probably ```kinc``` is only useful as example code

## Technical Example
As an example, the K rule for unrolling a simple ```while``` construct looks like this:

```
rule <k> (while(B, S) => ifThenElse(B, kra(S ~> while(B, S)), kra())) ~> K:listk </k>
```

It is then compiled into the following C code:

```c
int rule39(Configuration* config) {
	// checking CheckNumArgs: k must have at least 1 arguments
	if (k_length(config->k) < 1) { return 1; }

	// checking CheckNumArgs: k.0 must have 2 arguments
	if (k_num_args(k_get_item(config->k, 0)) != 2) { return 1; }

	// checking CheckLabel: k.0 must have the 'while label
	if (k_get_item(config->k, 0)->label->type != e_symbol) { return 1; }
	if (k_get_item(config->k, 0)->label->symbol_val != symbol_while) { return 1; }
	K* oldK = config->k->holder;
	Inc(oldK);

	K* variable_B = k_get_arg(k_get_item(config->k, 0), 0);
	K* variable_S = k_get_arg(k_get_item(config->k, 0), 1);
	K* variable_K = computation_without_first_n_arg(config->k, 1);

	// Replacement: 1 elements at k.0 should be replaced with [ifThenElse(B:k,statements(S:k,while(B:k,S:k)),statements())]
	K* arg_arg_0 = variable_B; // isList = false
	K* arg_1_arg_0 = variable_S; // isList = false
	K* arg_1_1_arg_0 = variable_B; // isList = false
	K* arg_1_1_arg_1 = variable_S; // isList = false
	K* arg_1_arg_1 = k_new(SymbolLabel(symbol_while), 2, arg_1_1_arg_0, arg_1_1_arg_1); // isList = false
	K* arg_arg_1 = k_new(SymbolLabel(symbol_statements), 2, arg_1_arg_0, arg_1_arg_1); // isList = false
	K* arg_arg_2 = k_new_empty(SymbolLabel(symbol_statements)); // isList = false
	computation_insert_elems(config->k, 0, 1, 1, 1, E_NOT_LIST, k_new(SymbolLabel(symbol_ifThenElse), 3, arg_arg_0, arg_arg_1, arg_arg_2));
	// Cleanup:
	k_dispose(variable_K);
	Dec(oldK);

	return 0;
}
```

Much can be simplified, but it was fast enough for a demo, which was the goal.
