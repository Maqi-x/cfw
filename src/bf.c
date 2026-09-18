#include <bf.h>
#include <defs.h>

#include <stdlib.h>

#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_mutex.h>

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

bool RunBrainfuck(Instruction* prog, BfIoState* out) {
    uchar* data = calloc(DATA_SIZE, 1);
    if (data == NULL) return false;

    uint dp = 0;
    uint pc = 0;
    bool success = true;

    while (prog[pc].operator != OP_END) {
        if (out->stop) {
            success = false;
            break;
        }

        switch (prog[pc].operator) {
        case OP_INC_DP:
            dp = (dp + 1) % DATA_SIZE;
            break;
        case OP_DEC_DP:
            dp = (dp == 0) ? DATA_SIZE - 1 : dp - 1;
            break;
        case OP_INC_VAL:
            data[dp]++;
            break;
        case OP_DEC_VAL:
            data[dp]--;
            break;

        case OP_OUT:
            SDL_LockMutex(out->m);
            if (out->outLen < BF_OUT_BUF_SIZE) {
                out->outBuf[out->outLen++] = data[dp];
            }
            SDL_UnlockMutex(out->m);
            break;
        case OP_IN:
            // TODO
            unreachable();

        case OP_JMP_FWD:
            if (data[dp] == 0) {
                pc = prog[pc].operand;
            }
            break;
        case OP_JMP_BCK:
            if (data[dp] != 0) {
                pc = prog[pc].operand;
            }
            break;
        }

        if (!success) break;
        pc++;
    }

    free(data);
    return success;
}

