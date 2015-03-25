import sys

n = sys.argv[1]
s = 0
while (not(n <= 0)):
	s = s + n
	n = n + -1
print s
