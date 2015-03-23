.DELETE_ON_ERROR:

DIST = ../../dist

VISITORS = v_variable_types.go v_symbol_labels.go v_constraints.go v_subsorts.go
DEPS = $(VISITORS) kinc_t.go parser.go kinc.nn.go visitor.go kinc_print.go kinc.go pretty_print.go checks.go replacements.go bindings.go reference.go compile.go

all: kinc

dist: kinc
	mkdir -p $(DIST)/bin && cp ./kinc $(DIST)/bin

kinc: $(DEPS)
	go build -o kinc $(DEPS)

parser.go y.output: kinc.nn.go parser.y kinc_t.go
	go tool yacc -o="parser.go" -v="y.output" parser.y
# -p="Kinc" 

kinc.nn.go: kinc.nex
	/tmp/go/bin/nex kinc.nex
