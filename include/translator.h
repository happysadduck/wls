#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "core.h"

void translator_prepare(void);
int parse_keyword(const char *cmd);
char *parse_prop_def(const char *prop_def_cmd, char *prop_mem, char *top);
char *parse_prop_inv(char *prop_inv_cmd, char *cmd_end, char *top);
int context_is_end(void);

#endif
