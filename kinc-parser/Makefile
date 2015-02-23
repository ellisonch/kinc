.DELETE_ON_ERROR:

all: test

test: parser.go
#	go build &&
	go test -v

parser.go y.output: lexer.go parser.y kinc.go
	go tool yacc -p="Kinc" -o="parser.go" -v="y.output" parser.y
