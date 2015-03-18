package main

import "fmt"
import "os"
import "strings"
import "errors"
import "io"
import "flag"

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



func main() {
	// wordPtr := flag.String("word", "foo", "a string")
	// numbPtr := flag.Int("numb", 42, "an int")
	// boolPtr := flag.Bool("fork", false, "a bool")
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "Usage: kinc [options] language.kinc\n\n")
		flag.PrintDefaults()
	}

	flag.Parse()

	trailingArgs := flag.Args()
	
	var kincFile string

	if len(trailingArgs) == 1 {
		kincFile = trailingArgs[0]
	} else {
		flag.Usage()
		fmt.Fprintf(os.Stderr, "You must mention exactly one .kinc file at the end of the arguments\n")
		os.Exit(1)
	}

	lang, err := ParseFile(kincFile)
	if err != nil {
		fmt.Printf("Couldn't parse language: %s\n", err)
		os.Exit(1)
	}

	c := lang.Compile()
	fmt.Printf("%s\n", c)

	os.Exit(0)
}
