#ifndef MACROS_H_
#define MACROS_H_

#ifdef UNITTEST
#define PRIVATE
#define INLINE
#else
#define PRIVATE static
#define INLINE  inline
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif  // MACROS_H_
