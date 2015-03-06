package main

import "os"
import "fmt"
import "strings"
import "os/exec"
import "io/ioutil"
import "bytes"

func runTest(testName string, fileName string, expected []byte) (passed bool) {
	program := "./" + testName + ".exe"
	fullpath := testName + "/" + fileName + ".aterm"

	cmd := exec.Command(program, fullpath)
	// cmd.Stdin = strings.NewReader("some input")
	var out bytes.Buffer
	cmd.Stdout = &out
	err := cmd.Run()
	if err != nil {
		panic(err)
	}

	if bytes.Compare(out.Bytes(), expected) != 0 {
		fmt.Fprintf(os.Stderr, "Suite \"%s\", Test \"%s\"\nExpected:\n%s\nSaw:\n%s\n", testName, fileName, expected, out.Bytes())
		return false
	} else {
		return true
	}
}

func main() {
	dirname := os.Args[1]

	tests := []string{}
	results := make(map[string][]byte)

	// from http://stackoverflow.com/questions/6608873/file-system-scanning-in-golang
	d, err := os.Open(dirname)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	defer d.Close()
	fi, err := d.Readdir(-1)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	for _, fi := range fi {
		if !fi.Mode().IsRegular() {
			continue;
		}
		if strings.HasSuffix(fi.Name(), ".aterm") {
			base := strings.TrimSuffix(fi.Name(), ".aterm")
			tests = append(tests, base)
			continue;
		}
		if strings.HasSuffix(fi.Name(), ".result") {
			base := strings.TrimSuffix(fi.Name(), ".result")
			dat, err := ioutil.ReadFile(dirname + "/" + base + ".result")
			if err != nil {
				panic(err)
			}
			results[base] = dat
		}
		// fmt.Println(fi.Name(), fi.Size(), "bytes")
	}

	validTests := 0
	passedTests := 0

	for _, test := range tests {
		if expected, ok := results[test]; ok {
			validTests++
			if runTest(dirname, test, expected) {
				passedTests++
			}
		} else {
			fmt.Fprintf(os.Stderr, "Test \"%s\" doesn't have a results file\n")
		}
	}

	fmt.Printf("Passed %d of %d\n", passedTests, validTests)
	if validTests == passedTests {
		os.Exit(0)
	} else {
		os.Exit(1)
	}
}