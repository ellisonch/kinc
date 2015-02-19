a = 1100087778366101931
b = 7540113804746346429
while (not(a <= b and b <= a)):
	if (b <= a):
		a = a - b
	else:
		b = b - a
print(a)
