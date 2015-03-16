# int m, n, q, r, s ;
s = 0
m = 2000
while (not(m <= 2)):
	n = m
	m = m + -1
	while (not(n <= 1)):
		s = s + 1
		q = n / 2
		r = q + q + 1
		if (r <= n):
			n = n + n + n + 1
		else:
			n = q

print s