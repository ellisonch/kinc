.DELETE_ON_ERROR:

VISITORS=v_variable_types.go v_symbol_labels.go v_constraints.go v_subsorts.go
DEPS=$(VISITORS) kinc_t.go parser.go kinc.nn.go visitor.go kinc_print.go kinc.go pretty_print.go checks.go replacements.go bindings.go reference.go compile.go


all: run

imp: kinc.exe
	./kinc.exe ../tests/imp/imp.kinc

run: kinc.exe
	./kinc.exe

kinc.exe: $(DEPS)
	go build -o kinc.exe $(DEPS)

# compiler-test: compiler_test.go kinc_t.go parser.go kinc.nn.go visitor.go kinc_print.go
# 	go test -v compiler_test.go kinc_t.go parser.go kinc.nn.go visitor.go kinc_print.go

test: parser.go
#	go build &&
	go test -v

parser.go y.output: kinc.nn.go parser.y kinc_t.go
	go tool yacc -o="parser.go" -v="y.output" parser.y
# -p="Kinc" 

kinc.nn.go: kinc.nex
	/tmp/go/bin/nex kinc.nex
