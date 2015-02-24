.DELETE_ON_ERROR:

all: test

test: parser.go
#	go build &&
	go test -v

parser.go y.output: kinc.nn.go parser.y kinc.go
	go tool yacc -o="parser.go" -v="y.output" parser.y
# -p="Kinc" 

kinc.nn.go: kinc.nex
	/tmp/go/bin/nex kinc.nex
