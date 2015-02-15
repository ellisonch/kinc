run: aterm2c.exe gen_java/ImpUglyParser.class
	cat ../programs/prog1.imp
	cat ../programs/prog1.imp | ./grun ImpUgly program 
	cat ../programs/prog1.imp | ./grun ImpUgly program | ./aterm2c.exe

aterm2c.exe: aterm2c.go
	go build aterm2c.go

gen_antlr/ImpUglyParser.java: ASTVisitor.java ImpUgly.g4
	mkdir -p gen_java
	./antlr4 -o gen_antlr ImpUgly.g4
	
gen_java/ImpUglyParser.class: gen_antlr/ImpUglyParser.java
	javac -d gen_java *.java gen_antlr/*.java

