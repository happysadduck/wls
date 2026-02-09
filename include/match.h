# ifndef MATCH_H
# define MATCH_H

# include"token.h"
# include"hash.h"

# define CARPET_REPLACE_STR(t, src, dst)\
do{\
    Token raw, out;\
    raw.start=src;\
    raw.len=strlen(raw.start);\
    out.start=dst;\
    carpet_replace(t, &raw, &out);\
}while(0)

void batch_replace(
				   Tokens*params, 
				   Tokens*new_params, 
				   const char*src,
				   char*out);
void carpet_replace(
                    HashTower*t,
                    Token*src,
                    Token*out);

# endif
