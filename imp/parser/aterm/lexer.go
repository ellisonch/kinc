package aterm

import "bufio"
import "io"
import "log"
import "fmt"
import "strconv"

func go_sucks_lexer() {
	_ = fmt.Printf
}

const TOK_EOF = 0

var tok_names map[int]string = map[int]string{
	TOK_ERR: "TOK_ERR",
	TOK_EOF: "TOK_EOF",
	',': ",",
	'(': "(",
	')': ")",
	'{': "{",
	'}': "}",
	'[': "[",
	']': "]",
	TOK_STRING: "TOK_STRING",
	TOK_CONSTRUCTOR: "TOK_CONSTRUCTOR",
	TOK_INTEGER: "TOK_INTEGER",
	TOK_REAL: "TOK_REAL",
}

type ATermLex struct {
	stream* bufio.Reader
	currToken int
}

func NewATermLexer(s io.Reader) *ATermLex {
	l := new(ATermLex)
	l.stream = bufio.NewReader(s)
	return l
}

func (l *ATermLex) LexString(lval *ATermSymType) int {
	var s []rune

	for {
		r, size, err := l.stream.ReadRune()
		_ = size
		if err == io.EOF {
			return TOK_ERR
		}
		if err != nil {
			log.Fatalf("Error reading Rune: %v\n", err)
		}
		if r == '"' {
			lval.str = string(s)
			return TOK_STRING
		}
		s = append(s, r)
	}

}

func (l *ATermLex) LexConstructor(start rune, lval *ATermSymType) int {
	var s []rune = []rune{start}

	for {
		r, size, err := l.stream.ReadRune()
		_ = size
		if err != nil {
			l.stream.UnreadRune()
			lval.str = string(s)
			return TOK_CONSTRUCTOR
		}
		if ((r >= 'a' && r <= 'z') || (r >= 'A' && r <= 'Z')) || (r >= '0' && r <= '9') {
			s = append(s, r)
		} else {
			l.stream.UnreadRune()
			lval.str = string(s)
			return TOK_CONSTRUCTOR
		}
	}
}

func (l *ATermLex) LexInt(start rune, lval *ATermSymType) int {
	var s []rune = []rune{start}

	for {
		r, size, err := l.stream.ReadRune()
		_ = size
		if err != nil {
			l.stream.UnreadRune()
			i64, err := strconv.ParseInt(string(s), 10, 64)
			if err != nil { return TOK_ERR }
			lval.i64 = i64
			return TOK_INTEGER
		}
		// fmt.Printf("%v\n", r)
		if (r >= '0' && r <= '9') {
			s = append(s, r)
		} else {
			l.stream.UnreadRune()
			i64, err := strconv.ParseInt(string(s), 10, 64)
			if err != nil { return TOK_ERR }
			lval.i64 = i64
			// fmt.Printf("%d\n", lval.i64)
			return TOK_INTEGER
		}
	}
}

func (l *ATermLex) TokenList() []int {
	tokens := make([]int, 0)
	var info *ATermSymType = new(ATermSymType)
	for {
		token := l.Lex(info)
		tokens = append(tokens, token)
		if token == TOK_EOF {
			break
		}
	}
	return tokens
}

func (l *ATermLex) Lex(lval *ATermSymType) int {
	for {
		r, size, err := l.stream.ReadRune()
		_ = size
		if err == io.EOF {
			return TOK_EOF
		}
		if err != nil {
			log.Fatalf("Error reading Rune: %v\n", err)
		}	
		// fmt.Printf("%s (%d)\n", string(r), r)

		switch {
			case r == ',': return ','
			case r == '(': return '('
			case r == ')': return ')'
			case r == '{': return '{'
			case r == '}': return '}'
			case r == '[': return '['
			case r == ']': return ']'
			case r == '"': return l.LexString(lval)
			case (r >= 'A' && r <= 'Z') || (r >= 'a' && r <= 'z') || r == '#': return l.LexConstructor(r, lval)
			case r >= '0' && r <= '9': return l.LexInt(r, lval)
			default: return TOK_ERR
			case r == ' ', r == '\n', r == '\r', r == '\t': // just read another token
		}

	}
		
	return TOK_ERR
}

func (l *ATermLex) Error(s string) {
	fmt.Printf("Syntax error: %s\n", s)
}