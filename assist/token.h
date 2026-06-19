#ifndef TOKEN_H
#define TOKEN_H

#include "ptr.h"

typedef struct {
    moff_t start;
    cnt_t len;
} token_t;

typedef struct {
    moff_t tokens;
    cnt_t cnt;
} tokens_t;

const char *match_right_paren(const char *p);
const char *strstr_with_skipping_brackets(const char *src, const char *sub);
void remove_outer_brackets(token_t *token);
int token_to_tokens(token_t token, const char *wall, tokens_t *out,
                    cnt_t max_token_cnt);
char *token_to_str(token_t token, char *out, char *top);
char token_cut(token_t token);
void token_restore(token_t token, char removed);

#endif
