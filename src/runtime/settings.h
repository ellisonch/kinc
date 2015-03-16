#ifndef SETTINGS_H
#define SETTINGS_H

#define MAX_STATE 26
#define MAX_K 20
// #define MAX_GARBAGE_PENDING 100
#define MAX_GARBAGE_KEPT 10000
#define MAX_GARBAGE_ARG_LEN 64
#define SYMBOLS_MAX 1024
#define MAX_OUTSTANDING_K 5000

// when printing k terms, print the ref counts as well
#ifndef printRefCounts
	#define printRefCounts 0
#endif

#ifndef printDebug
	#define printDebug 0
#endif

// slow checks dealing with refcounting
#define shouldCheck 0

// technically not needed, but good to be safe
#define checkRightSettings 1
#define checkTypeSafety 1
#define checkRefCounting 1
#define checkGC 1
#define checkTermSize 1
#define checkStackSize 1
#define helpSafety 1

#endif
