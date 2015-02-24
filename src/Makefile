.DELETE_ON_ERROR:

all: compiler-test

compiler-test: compiler_test.go kinc_t.go parser.go kinc.nn.go  
	go test -v compiler_test.go kinc_t.go parser.go kinc.nn.go

test: parser.go
#	go build &&
	go test -v

parser.go y.output: kinc.nn.go parser.y kinc_t.go
	go tool yacc -o="parser.go" -v="y.output" parser.y
# -p="Kinc" 

kinc.nn.go: kinc.nex
	/tmp/go/bin/nex kinc.nex
