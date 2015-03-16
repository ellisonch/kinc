all:
	go tool yacc -p="ATerm" -o="parser.go" parser.y && echo "yacced" && go build && go test
