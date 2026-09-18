#include <bf.h>
#include <defs.h>

#include <stdlib.h>

#include <string.h>

// NOTE: Stolen from https://github.com/kgabis/brainfuck-c
//       Credits to the original author: @kgabis
//       Licensed under the MIT license
//
//       Changes:
//         rename some functions, variables and types
//         accept string argument instead of FILE*
//         don't rely on global state

#define OP_END          0
#define OP_INC_DP       1
#define OP_DEC_DP       2
#define OP_INC_VAL      3
#define OP_DEC_VAL      4
#define OP_OUT          5
#define OP_IN           6
#define OP_JMP_FWD      7
#define OP_JMP_BCK      8

#define STACK_SIZE      512
#define DATA_SIZE       65535

#define STACK_PUSH(A)   (stack[sp++] = A)
#define STACK_POP()     (stack[--sp])
#define STACK_EMPTY()   (sp == 0)
#define STACK_FULL()    (sp == STACK_SIZE)

bool CompileBrainfuck(const char* code, Instruction prog[static PROGRAM_SIZE]) {
    if (code == NULL) return false;

    uint pc = 0, jmp_pc;
    uint stack[STACK_SIZE];
    uint sp = 0;

    usize i = 0;
    int c;
    while ((c = code[i++]) != '\0' && pc < PROGRAM_SIZE) {
        switch (c) {
            case '>': prog[pc].operator = OP_INC_DP; break;
            case '<': prog[pc].operator = OP_DEC_DP; break;
            case '+': prog[pc].operator = OP_INC_VAL; break;
            case '-': prog[pc].operator = OP_DEC_VAL; break;
            case '.': prog[pc].operator = OP_OUT; break;
            case ',': prog[pc].operator = OP_IN; break;
            case '[':
                prog[pc].operator = OP_JMP_FWD;
                if (STACK_FULL()) {
                    return false;
                }
                STACK_PUSH(pc);
                break;
            case ']':
                if (STACK_EMPTY()) {
                    return false;
                }
                jmp_pc = STACK_POP();
                prog[pc].operator = OP_JMP_BCK;
                prog[pc].operand = jmp_pc;
                prog[jmp_pc].operand = pc;
                break;
            default: pc--; break;
        }
        pc++;
    }
    if (!STACK_EMPTY() || pc == PROGRAM_SIZE) {
        return false;
    }
    prog[pc].operator = OP_END;
    return true;
}

// Stolen code ends here.

void InitBrainfuck(BfContext* ctx) {
    memset(ctx->data, 0, sizeof(ctx->data));
    ctx->pc = 0;
    ctx->dp = 0;
    ctx->outLen = 0;
    ctx->inLen = 0;
    ctx->inPos = 0;
    ctx->running = true;
}

bool StepBrainfuck(Instruction* prog, BfContext* ctx, usize maxSteps) {
    if (!ctx->running) return false;

    usize steps = 0;
    while (prog[ctx->pc].operator != OP_END && steps < maxSteps) {
        switch (prog[ctx->pc].operator) {
        case OP_INC_DP:
            ctx->dp = (ctx->dp + 1) % DATA_SIZE;
            break;
        case OP_DEC_DP:
            ctx->dp = (ctx->dp == 0) ? DATA_SIZE - 1 : ctx->dp - 1;
            break;
        case OP_INC_VAL:
            ctx->data[ctx->dp]++;
            break;
        case OP_DEC_VAL:
            ctx->data[ctx->dp]--;
            break;

        case OP_OUT:
            if (ctx->outLen < BF_OUT_BUF_SIZE) {
                ctx->outBuf[ctx->outLen++] = ctx->data[ctx->dp];
            }
            break;
        case OP_IN:
            // TODO
            unreachable();

        case OP_JMP_FWD:
            if (ctx->data[ctx->dp] == 0) {
                ctx->pc = prog[ctx->pc].operand;
            }
            break;
        case OP_JMP_BCK:
            if (ctx->data[ctx->dp] != 0) {
                ctx->pc = prog[ctx->pc].operand;
            }
            break;
        }

        ctx->pc++;
        steps++;
    }

    if (prog[ctx->pc].operator == OP_END) {
        ctx->running = false;
    }

    return ctx->running;
}

bool RunBrainfuck(Instruction* prog, BfContext* ctx) {
    InitBrainfuck(ctx);
    return StepBrainfuck(prog, ctx, (usize)-1);
}

