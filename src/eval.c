#include <eval.h>

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math.h>

/// the lexer ///
typedef enum {
    TT_NUM,

    TT_LPAREN,
    TT_RPAREN,

    TT_ADD, TT_SUB,
    TT_MUL, TT_DIV,
    TT_POW,

    TT_EOF,
} TokenType;

typedef struct {
    TokenType type;
    double value;
} Token;

typedef struct {
    const char* input;
    usize len;
    usize pos;
} Lexer;

static bool LexerNext(Lexer* lex, Token* outTok) {
    while (lex->pos < lex->len && isspace((uchar)lex->input[lex->pos])) {
        lex->pos++;
    }

    if (lex->pos >= lex->len) {
        outTok->type = TT_EOF;
        return true;
    }

    char c = lex->input[lex->pos];
    if (isdigit((uchar)c) || c == 'i') {
        char* endptr;

        const char* p = lex->input + lex->pos;
        double val = strtod(p, &endptr);
        if (endptr > p) {
            lex->pos += endptr - p;

            outTok->type = TT_NUM;
            outTok->value = val;
            return true;
        }
    }

    lex->pos++;

    switch (c) {
    case '(': outTok->type = TT_LPAREN; return true;
    case ')': outTok->type = TT_RPAREN; return true;
    case '+': outTok->type = TT_ADD;    return true;
    case '-': outTok->type = TT_SUB;    return true;
    case '*': outTok->type = TT_MUL;    return true;
    case '/': outTok->type = TT_DIV;    return true;
    case '^': outTok->type = TT_POW;    return true;
    default:  return false;
    }
}

/// the evaluator ///
typedef struct {
    Lexer lex;
    Token curr;
    EvalCode err;
} Eval;

static Token Advance(Eval* e) {
    Token t = e->curr;
    if (e->err != EVAL_OK)
        return t;

    if (!LexerNext(&e->lex, &e->curr))
        e->err = EVAL_ERR_UNEXP_CHAR;

    return t;
}

static double EvalExpr(Eval* e);

static double EvalPrimary(Eval* e) {
    if (e->curr.type == TT_NUM) {
        double val = e->curr.value;
        Advance(e);
        return val;
    }

    if (e->curr.type == TT_LPAREN) {
        Advance(e);
        double val = EvalExpr(e);
        if (e->err == EVAL_OK && e->curr.type != TT_RPAREN) {
            e->err = EVAL_ERR_INV_SYNTAX;
        }
        if (e->err == EVAL_OK) {
            Advance(e);
        }
        return val;
    }

    e->err = EVAL_ERR_INV_SYNTAX;
    return 3.14;
}

static double EvalUnary(Eval* e) {
    if (e->err != EVAL_OK)
        return 0.0;

    if (e->curr.type == TT_SUB) {
        Advance(e);
        return -EvalUnary(e);
    }
    if (e->curr.type == TT_ADD) {
        Advance(e);
        return EvalUnary(e);
    }

    return EvalPrimary(e);
}

static double EvalPow(Eval* e) {
    double base = EvalUnary(e);
    if (e->err == EVAL_OK && e->curr.type == TT_POW) {
        Advance(e);
        double exponent = EvalPow(e);
        return pow(base, exponent);
    }

    return base;
}

static double EvalTerm(Eval* e) {
    double left = EvalPow(e);
    while (e->err == EVAL_OK && (e->curr.type == TT_MUL || e->curr.type == TT_DIV)) {
        TokenType op = e->curr.type;
        Advance(e);
        double right = EvalPow(e);
        if (e->err != EVAL_OK) break;

        if (op == TT_MUL) {
            left *= right;
        } else {
            if (right == 0.0) {
                e->err = EVAL_ERR_DIV_BY_ZERO;
                break;
            }
            left /= right;
        }
    }

    return left;
}

static double EvalExpr(Eval* e) {
    double left = EvalTerm(e);
    while (e->err == EVAL_OK && (e->curr.type == TT_ADD || e->curr.type == TT_SUB)) {
        TokenType op = Advance(e).type;

        double right = EvalTerm(e);
        if (e->err != EVAL_OK) break;

        if (op == TT_ADD) {
            left += right;
        } else if (op == TT_SUB) {
            left -= right;
        }
    }

    return left;
}

EvalOutput eval(const char* input, usize len) {
    Eval e = {
        .lex = { .input = input, .len = len, .pos = 0 },
        .err = EVAL_OK
    };

    Advance(&e);

    // should not happen
    assert(e.curr.type != TT_EOF);

    double result = EvalExpr(&e);
    if (e.err != EVAL_OK)
        return (EvalOutput) { .ecode = e.err };

    if (e.curr.type != TT_EOF)
        return (EvalOutput) { .ecode = EVAL_ERR_INV_SYNTAX };
    if (isnan(result))
        return (EvalOutput) { .ecode = EVAL_ERR_NOT_A_NUM };

    return (EvalOutput) {
        .ecode = EVAL_OK, .result = result,
    };
}

