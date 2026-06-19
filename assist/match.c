#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "match.h"

/**
 * @brief 将token fact 所对应字符串中所有的 paras 所记录的子串变为 new_paras
 * 所记录的字串. 输入的 paras 和 new_paras 必须严格具备相同大小的 cnt,
 * 并且其中每一个指向的字符串 moff_t 都有效 (而不是0)
 */
char *replace_braces(const token_t *fact, const tokens_t *paras,
                     const tokens_t *new_paras, char *out, const char *top) {
    char *fact_start;
    cnt_t fact_len;
    char *out_ptr;
    cnt_t pos;
    cnt_t end;
    cnt_t inner_len;
    cnt_t i;
    int matched;
    token_t *p;
    token_t *rep;

    fact_start = PTR(fact->start);
    fact_len = fact->len;
    out_ptr = out;

    pos = 0;
    while (pos < fact_len) {
        if (fact_start[pos] == '{') {
            end = pos + 1;
            while (end < fact_len && fact_start[end] != '}')
                end++;
            if (end >= fact_len) {
                if (out_ptr + 1 > top)
                    return NULL;
                *out_ptr++ = '{';
                pos++;
                continue;
            }
            inner_len = end - pos - 1;
            matched = -1;
            for (i = 0; i < paras->cnt; i++) {
                token_t *ptokens = PTR(paras->tokens);
                char *para_start;
                assert(para_start);
                p = &(ptokens[i]);
                para_start = PTR(p->start);
                if (inner_len == p->len &&
                    memcmp(fact_start + pos + 1, para_start, inner_len) == 0) {
                    matched = i;
                    break;
                }
            }
            if (matched != -1 && matched < new_paras->cnt) {
                token_t *nptokens = PTR(new_paras->tokens);
                char *rep_start;
                assert(rep_start);
                rep = &(nptokens[matched]);
                rep_start = PTR(rep->start);
                if (out_ptr + rep->len + 1 > top)
                    return NULL;

                memcpy(out_ptr, rep_start, rep->len);
                out_ptr += rep->len;
            } else {
                if (out_ptr + inner_len + 3 > top)
                    return NULL;
                *out_ptr++ = '{';
                memcpy(out_ptr, fact_start + pos + 1, inner_len);
                out_ptr += inner_len;
                *out_ptr++ = '}';
            }
            pos = end + 1;
        } else {
            if (out_ptr + 1 > top)
                return NULL;
            *out_ptr++ = fact_start[pos];
            ++pos;
        }
    }
    if (out_ptr + 1 == top)
        return NULL;
    *out_ptr = '\0';
    return out_ptr;
}

char *construct_param_name(cnt_t *cons_para_cnt, char *out, char *top) {
    char num[12];
    sprintf(num, "%d", (*cons_para_cnt)++);
    strncpy(out, " ", top - out - 1);
    if (!out)
        return NULL;
    strcat(out, num);
    return out + strlen(out);
}

/**
 * @brief 将 src 变为 !(src), 写入 out 中. 如果超出 top 则返回NULL
 * @return 构造的 !(src) 字符串的顶端 '\0' 字符.
 */
char *construct_nagtive_suppose(const char *src, char *out, const char *top) {
    cnt_t len = strlen(src);
    if (top < out + len + 3)
        return NULL;
    out[0] = '!';
    out[1] = '(';
    memcpy(out + 2, src, len);
    out[2 + len] = ')';
    out[3 + len] = '\0';
    return out + len + 3;
}
