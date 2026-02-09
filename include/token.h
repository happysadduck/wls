# ifndef TOKEN_H
# define TOKEN_H

# include<string.h>

# include"config.h"

typedef struct{
	const char*start;
	int len;
}Token;

typedef struct{
	Token tokens[MAX_TOKENS];
	int cnt;
}Tokens;

const char*match_right_paren(const char*p);
const char*strstr_with_skipping_brackets(
												const char*src, const char*sub);
void remove_outer_brackets(Token*token);
int token_to_tokens(
					   Token token, const char*wall, Tokens*out);
void print_token(Token token);
void token_to_str(Token*token, char*out);

# endif
