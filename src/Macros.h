#ifndef MACROS_H
#define MACROS_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#ifdef UNITTEST
#define PRIVATE
#else
#define PRIVATE static
#endif

#endif
