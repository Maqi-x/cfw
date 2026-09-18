#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <defs.h>

typedef struct SDL_Mutex SDL_Mutex;
typedef struct SDL_Condition SDL_Condition;

// the output view in bf app does not support scrolling
// anyway so having a bigger buffer is pointless you won't
// see rest of the characters anyway
#define BF_OUT_BUF_SIZE 512

// I'm not sure if they even need to be powers of two
// but maybe it will help the compiler with optimizing
// stuff or something
#define BF_IN_BUF_SIZE  1024

typedef struct {
    // out
    char outBuf[BF_OUT_BUF_SIZE];
    usize outLen;

    // in
    char inBuf[BF_IN_BUF_SIZE];
    usize inLen;
    usize inPos;

    // stop flag
    _Atomic bool stop;

    // very descriptive names
    SDL_Mutex*     m;
    SDL_Condition* c;
} BfIoState;

typedef struct {
    uint operator;
    uint operand;
} Instruction;

#define PROGRAM_SIZE 4096

bool CompileBrainfuck(const char* code, Instruction prog[static PROGRAM_SIZE]);
bool RunBrainfuck(Instruction* prog, BfIoState* out);

