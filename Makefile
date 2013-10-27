run:
	 go build imp.go && cp impParser/ImpUgly*.class . && time cat prog2.imp | grun ImpUgly program | cat prog2.aterm | ./imp.exe
