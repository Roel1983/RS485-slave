#ifndef MACROS_H
#define MACROS_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#ifdef UNITTEST
#define PRIVATE
#define INLINE
#else
#define PRIVATE static
#define INLINE  inline
#endif

#endif
