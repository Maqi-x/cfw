#pragma once
#include <stddef.h>

typedef size_t usize;
typedef unsigned int uint;
typedef unsigned char uchar;

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define vlen VECTOR_SIZE

// i just learned that C23 has unreachable() macro
// in the standard library, so we need to guard this
// with #ifndef to prevent redefinition warnings
#ifndef unreachable
    #define unreachable() __builtin_unreachable()
#endif

typedef struct TTF_TextEngine TTF_TextEngine;
extern TTF_TextEngine* tengine;

