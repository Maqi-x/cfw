#pragma once

#include <stdbool.h>
#include <defs.h>

// the output view in bf app does not support scrolling
// anyway so having a bigger buffer is pointless you won't
// see rest of the characters anyway
#define BF_OUT_BUF_SIZE 512

// I'm not sure if they even need to be powers of two
// but maybe it will help the compiler with optimizing
// stuff or something
#define BF_IN_BUF_SIZE  1024

#define DATA_SIZE       65535
#define PROGRAM_SIZE    4096

typedef struct {
    uint operator;
    uint operand;
} Instruction;

typedef struct {
    // out
    char outBuf[BF_OUT_BUF_SIZE];
    usize outLen;

    // in
    char inBuf[BF_IN_BUF_SIZE];
    usize inLen;
    usize inPos;

    // vm execution state
    uchar data[DATA_SIZE];
    uint pc;
    uint dp;
    bool running;
} BfContext;

bool CompileBrainfuck(const char* code, Instruction prog[static PROGRAM_SIZE]);
void InitBrainfuck(BfContext* ctx);
bool StepBrainfuck(Instruction* prog, BfContext* ctx, usize maxSteps);
bool RunBrainfuck(Instruction* prog, BfContext* ctx);

