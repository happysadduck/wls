#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>

#include "ptr.h"

char *get_cmd(FILE *fp, char *out, char *top, int *end_with_colon,
              int *is_teriminal, cnt_t *lines_span);

#endif
