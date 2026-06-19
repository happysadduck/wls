#include <stdio.h>
#include <string.h>

#include "config.h"
#include "core.h"
#include "input.h"

char *get_cmd(FILE *fp, char *out, char *top, int *end_with_colon,
              int *is_terminal, cnt_t *lines_span) {
    int ch;
    int prev = 0;
    char *p = out;
    char *brace_start = NULL;
    *lines_span = 0;
    while ((ch = getc(fp)) != EOF) {
        if (ch == ' ')
            continue;
        if (ch == '#') {
            (*lines_span)++;
            if (prev == '.' || prev == ':') {
                p--;
                *p = '\0';
            }
            while ((ch = getc(fp)) != EOF && ch != '\n')
                ;
            *p = '\0';
            *end_with_colon = (prev == ':') ? 1 : 0;

            return p;
        }
        if (ch == '\n') {
            (*lines_span)++;
            if (prev == '.') {
                p--;
                *p = '\0';
                *end_with_colon = 0;

                return p;
            }
            if (prev == ':') {
                p--;
                *p = '\0';
                *end_with_colon = 1;

                return p;
            }
            continue;
        }
        if (ch == '[') {
            if (brace_start == NULL)
                brace_start = p;
            if (p > top)
                return NULL;
            *p++ = '[';
            prev = '[';
            continue;
        }
        if (ch == ']') {
            if (brace_start != NULL) {
                cnt_t content_len = p - brace_start - 1;
                char *replacement;
                *p = '\0';
                replacement =
                    tower_find(&CURR_SYSTEM.used_paras, brace_start + 1);
                if (!replacement) {
                    if (p > top)
                        return NULL;
                    *p++ = ']';
                    prev = ']';
                } else {
                    cnt_t old_len = p - brace_start;
                    cnt_t replace_len = strlen(replacement);
                    p = brace_start;
                    if (top - p < replace_len)
                        return NULL;
                    memcpy(p, replacement, replace_len);
                    p += replace_len;
                }
                brace_start = NULL;
            } else {
                if (p > top)
                    return NULL;
                *p++ = ']';
                prev = ']';
            }
            continue;
        }
        if (p > top)
            return NULL;
        *p++ = (char)ch;
        prev = ch;
    }
    *is_terminal = 1;
    *p = '\0';
    return p;
}
