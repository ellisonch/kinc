package main

import "fmt"
import "os"
import "strings"
import "errors"
import "io"

func ParseReader(r io.Reader) (*Language, error) {
	l := NewLexer(r)
	KincInit()
	ret := yyParse(l)

	if (ret == 0) {
		return Final, nil
	} else {
		return Final, errors.New("Kinc: error parsing string")
	}
}

func ParseString(s string) (*Language, error) {
	return ParseReader(strings.NewReader(s))	
}

func ParseFile(fname string) (*Language, error) {
	f, err := os.Open(fname)
	if (err != nil) {
		return nil, err
	}
	return ParseReader(f)
}

func safeForC(s string) string {
	// s = strings.Replace(s, "#", "_pound_", -1)
	return s
}

func main() {
	// fmt.Printf(prog)
	// l := NewLexer(strings.NewReader(prog))
	// KincInit()
	// ret := yyParse(l)
	// Final.String()

	argsWithoutProg := os.Args[1:]
	file := "../peano/peano.kinc"
	if len(argsWithoutProg) == 1 {
		file = argsWithoutProg[0]
	} else {
		panic("too many args")
	}

	// lang, err := ParseFile("../imp/imp.kinc")
	lang, err := ParseFile(file)
	// lang, err := ParseString(prog)
	if err != nil {
		fmt.Printf("Couldn't parse language: %s\n", err)
		os.Exit(1)
	}

	c := lang.Compile()

	// s := lang.PrettyPrint()
	// s := lang.String()
	fmt.Printf("%s\n", c)

	os.Exit(0)
}
