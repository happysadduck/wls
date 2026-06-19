#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "token.h"

const char *match_right_paren(const char *p) {
    const char *s;
    int depth = 1;
    if (!p || *p != '(')
        return NULL;
    for (s = p + 1; *s; s++) {
        if (*s == '(')
            depth++;
        else if (*s == ')') {
            depth--;
            if (depth == 0)
                return s;
        }
    }
    return NULL;
}

const char *strstr_with_skipping_brackets(const char *src, const char *sub) {
    const char *p, *bracket;
    p = src;
    do {
        bracket = strstr(p, "(");
        p = strstr(p, sub);
        if (!p)
            return NULL;
        if (!bracket || bracket >= p)
            break;
        bracket = match_right_paren(bracket);
        if (!bracket)
            return NULL;
        p = bracket + 1;
    } while (1);
    return p;
}

void remove_outer_brackets(token_t *token) {
    moff_t right_bracket;
    do {
        if (*(char *)PTR(token->start) != '(')
            return;
        right_bracket = OFF(match_right_paren(PTR(token->start)));
        if (right_bracket &&
            right_bracket - token->start + 1 - token->len == 0) {
            token->start++;
            token->len -= 2;
        } else
            return;
    } while (1);
}

int token_to_tokens(token_t token, const char *wall, tokens_t *out,
                    cnt_t max_tokens_cnt) {
    moff_t temp1, temp2;
    cnt_t i;
    token_t *tokens = PTR(out->tokens);
    out->cnt = 0;
    temp1 = token.start;
    i = 0;
    if (token.len == 0) {
        out->cnt = 0;
        return 1;
    }
    do {
        tokens[i].start = temp1;
        temp2 = OFF(strstr_with_skipping_brackets(PTR(temp1), wall));
        if (!temp2 || (temp2 - token.start) >= token.len) {
            tokens[i].len = token.len - (temp1 - token.start);
            remove_outer_brackets(tokens + i);
            if (tokens[i].len)
                i++;
            break;
        }
        tokens[i].len = temp2 - temp1;
        remove_outer_brackets(tokens + i);
        if (tokens[i].len)
            i++;
        temp1 = temp2 + strlen(wall);
    } while (i < max_tokens_cnt);
    if (i >= max_tokens_cnt)
        return 0;
    out->cnt = i;
    return 1;
}

/**
 * @brief 将 token 对应的字符串拷贝到 out 所在位置, 要求 token
 * 至少要有对应的字符串, 而不是0
 * @return 拷贝的新字符串的'\0'所在地址.
 */
char *token_to_str(token_t token, char *out, char *top) {
    char *token_start = PTR(token.start);
    assert(token_start);
    if (top - out < token.len)
        return NULL;
    memcpy(out, token_start, token.len);
    out[token.len] = '\0';
    return out + token.len;
}

char token_cut(token_t token) {
    char removed = *(char *)PTR(token.start + token.len);
    *(char *)PTR(token.start + token.len) = '\0';
    return removed;
}

void token_restore(token_t token, char removed) {
    *(char *)PTR(token.start + token.len) = removed;
}
