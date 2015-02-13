#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

_Noreturn void _panic(const char* func, const char* file, int line, const char* format, ...) {
	va_list va;
	va_start(va, format);
	fprintf(stderr, "PANIC! %s() (%s:%d): ", func, file, line);
	vfprintf(stderr, format, va);
	fprintf(stderr, "\n");
	exit(1);
}
