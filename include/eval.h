#pragma once
#include <defs.h>

typedef enum {
    EVAL_OK,
    EVAL_ERR_UNEXP_CHAR,
    EVAL_ERR_INV_SYNTAX,
    EVAL_ERR_DIV_BY_ZERO,
} EvalCode;

typedef struct {
    EvalCode ecode;
    double   result;
} EvalOutput;

EvalOutput eval(const char* input, usize len);

