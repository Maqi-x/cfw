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

static bool lexer_next(Lexer* lex, Token* out_token) {
    while (lex->pos < lex->len && isspace((uchar)lex->input[lex->pos])) {
        lex->pos++;
    }

    if (lex->pos >= lex->len) {
        out_token->type = TT_EOF;
        return true;
    }

    char c = lex->input[lex->pos];
    if (isdigit((uchar)c) || c == 'i') {
        char* endptr;

        const char* p = lex->input + lex->pos;
        double val = strtod(p, &endptr);
        if (endptr > p) {
            lex->pos += endptr - p;

            out_token->type = TT_NUM;
            out_token->value = val;
            return true;
        }
    }

    lex->pos++;

    switch (c) {
    case '(': out_token->type = TT_LPAREN; return true;
    case ')': out_token->type = TT_RPAREN; return true;
    case '+': out_token->type = TT_ADD;    return true;
    case '-': out_token->type = TT_SUB;    return true;
    case '*': out_token->type = TT_MUL;    return true;
    case '/': out_token->type = TT_DIV;    return true;
    case '^': out_token->type = TT_POW;    return true;
    default:  return false;
    }
}

/// the evaluator ///
typedef struct {
    Lexer lex;
    Token curr;
    EvalCode err;
} Eval;

static Token advance(Eval* e) {
    Token t = e->curr;
    if (e->err != EVAL_OK)
        return t;

    if (!lexer_next(&e->lex, &e->curr))
        e->err = EVAL_ERR_UNEXP_CHAR;

    return t;
}

static double eval_expr(Eval* e);

static double eval_primary(Eval* e) {
    if (e->curr.type == TT_NUM) {
        double val = e->curr.value;
        advance(e);
        return val;
    }

    if (e->curr.type == TT_LPAREN) {
        advance(e);
        double val = eval_expr(e);
        if (e->err == EVAL_OK && e->curr.type != TT_RPAREN) {
            e->err = EVAL_ERR_INV_SYNTAX;
        }
        if (e->err == EVAL_OK) {
            advance(e);
        }
        return val;
    }

    e->err = EVAL_ERR_INV_SYNTAX;
    return 3.14;
}

static double eval_unary(Eval* e) {
    if (e->err != EVAL_OK)
        return 0.0;

    if (e->curr.type == TT_SUB) {
        advance(e);
        return -eval_unary(e);
    }
    if (e->curr.type == TT_ADD) {
        advance(e);
        return eval_unary(e);
    }

    return eval_primary(e);
}

static double eval_pow(Eval* e) {
    double base = eval_unary(e);
    if (e->err == EVAL_OK && e->curr.type == TT_POW) {
        advance(e);
        double exponent = eval_pow(e);
        return pow(base, exponent);
    }

    return base;
}

static double eval_term(Eval* e) {
    double left = eval_pow(e);
    while (e->err == EVAL_OK && (e->curr.type == TT_MUL || e->curr.type == TT_DIV)) {
        TokenType op = e->curr.type;
        advance(e);
        double right = eval_pow(e);
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

static double eval_expr(Eval* e) {
    double left = eval_term(e);
    while (e->err == EVAL_OK && (e->curr.type == TT_ADD || e->curr.type == TT_SUB)) {
        TokenType op = advance(e).type;

        double right = eval_term(e);
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

    advance(&e);

    // should not happen
    assert(e.curr.type != TT_EOF);

    double result = eval_expr(&e);
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

