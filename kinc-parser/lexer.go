package kinc

import "bufio"
import "io"
import "log"
import "fmt"
import "strconv"
import "strings"

func go_sucks_lexer() {
	_ = fmt.Printf
}

const TOK_EOF = 0

// var tok_names map[int]string = map[int]string{
// 	TOK_ERR: "TOK_ERR",
// 	TOK_EOF: "TOK_EOF",
// 	',': ",",
// 	'(': "(",
// 	')': ")",
// 	':': ":",
// 	// '{': "{",
// 	// '}': "}",
// 	// '[': "[",
// 	// ']': "]",
// 	TOK_STRING: "TOK_STRING",
// 	TOK_CONSTRUCTOR: "TOK_CONSTRUCTOR",
// 	TOK_INTEGER: "TOK_INTEGER",
// 	TOK_REAL: "TOK_REAL",
// }

type KincLex struct {
	stream* bufio.Reader
	currToken int
}

func NewKincLexer(s io.Reader) *KincLex {
	l := new(KincLex)
	l.stream = bufio.NewReader(s)
	return l
}

func (l *KincLex) LexString(lval *KincSymType) int {
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

func (l *KincLex) LexConstructor(start rune) string {
	var s []rune = []rune{start}

	for {
		r, size, err := l.stream.ReadRune()
		_ = size
		if err != nil {
			l.stream.UnreadRune()
			return string(s)
			// lval.str = string(s)
			// return TOK_CONSTRUCTOR
		}
		if ((r >= 'a' && r <= 'z') || (r >= 'A' && r <= 'Z')) || (r >= '0' && r <= '9') {
			s = append(s, r)
		} else {
			l.stream.UnreadRune()
			return string(s)
			// lval.str = string(s)
			// return TOK_CONSTRUCTOR
		}
	}
}

func (l *KincLex) LexInt(start rune, lval *KincSymType) int {
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

func (l *KincLex) TokenList() []int {
	tokens := make([]int, 0)
	var info *KincSymType = new(KincSymType)
	for {
		token := l.Lex(info)
		tokens = append(tokens, token)
		if token == TOK_EOF {
			break
		}
	}
	return tokens
}

func (l *KincLex) Lex(lval *KincSymType) int {
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
			case r == '[': return '['
			case r == ']': return ']'
			case r == ':': return ':'
			case r == '<': {
				r2, size, err := l.stream.ReadRune()
				_ = size
				if err == io.EOF {
					return TOK_EOF
				}
				if err != nil {
					log.Fatalf("Error reading Rune: %v\n", err)
				}
				if r2 == '/' {
					return TOK_BEGIN_END
				}
				l.stream.UnreadRune()
				return '<'
			}
			case r == '>': return '>'
			case r == '=': {
				r2, size, err := l.stream.ReadRune()
				_ = size
				if err == io.EOF {
					return TOK_EOF
				}
				if err != nil {
					log.Fatalf("Error reading Rune: %v\n", err)
				}
				if r2 == '>' {
					return TOK_ARROW
				}
				l.stream.UnreadRune()
			}
			fallthrough
			// case r == '{': return '{'
			// case r == '}': return '}'
			// case r == '[': return '['
			// case r == ']': return ']'
			case r == '"': return l.LexString(lval)
			case (r >= 'A' && r <= 'Z') || (r >= 'a' && r <= 'z'): {
				ret := l.LexConstructor(r)
				lval.str = ret
				if (ret == "configuration") {
					return TOK_CONFIGURATION
				}
				if (ret == "rule") {
					return TOK_RULE
				}
				if (strings.ToUpper(string(r)) == string(r)) {
					return TOK_UC_NAME
				} else {
					return TOK_LC_NAME
				}
			}
			case r >= '0' && r <= '9': return l.LexInt(r, lval)
			default: return TOK_ERR
			case r == ' ', r == '\n', r == '\r', r == '\t': // just read another token
		}

	}
		
	return TOK_ERR
}

func (l *KincLex) ReportError() string {
	return fmt.Sprintf("Syntax error\n")
}

func (l *KincLex) Error(s string) {
	fmt.Printf("Syntax error: %s\n", s)
}