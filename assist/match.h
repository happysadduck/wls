#ifndef MATCH_H
#define MATCH_H

#include "token.h"

char *replace_braces(const token_t *fact, const tokens_t *paras,
                     const tokens_t *new_paras, char *out, const char *top);
char *construct_param_name(cnt_t *cons_para_cnt, char *out, char *top);
char *construct_nagtive_suppose(const char *src, char *out, const char *top);

#endif
