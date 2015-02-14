run:
	mkdir -p gen_java
	./antlr4 -o gen_antlr ImpUgly.g4 && javac -d gen_java *.java gen_antlr/*.java
	cat ../programs/prog1.imp | ./grun ImpUgly program

clean:
	rm *.class
