# var i, m, n, q, r, s, t, x, y, z ;
s = 0
m = 4000
n = 2
while (n <= m):
	i = 2
	q = n / i
	t = 1
	while (i <= q and 1 <= t):
		x = i
		y = q
		z = 0
		while (not(x <= 0)):
			q = x / 2
			r = q + q + 1
			if (r <= x):
				z = z + y
			x = q
			y = y + y
		if (n <= z):
			t = 0  
		else:
			i = i + 1
			q = n / i
	if (1 <= t):
		s = s + 1
	n = n + 1

print s