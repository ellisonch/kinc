#ifndef UTILS_H
#define UTILS_H

#define panic(...) (_panic(__func__, __FILE__, __LINE__, __VA_ARGS__))

_Noreturn void _panic(const char* func, const char* file, int line, const char* format, ...);

#endif
