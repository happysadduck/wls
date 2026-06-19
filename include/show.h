#ifndef SHOW_H
#define SHOW_H

#include "core.h"

void print_token(token_t token);
void print_tokens(tokens_t tokens);
void show_prop(prop_t *prop, char *indent_spaces);
void show_context(void);
void show_hash_tower(hash_tower_t *t);
void show_memory_screen(void);
void debug_screen(void);
void raise_error(char *error_msg);
void print_wls_report(void);

#endif
