# include<stdio.h>
# include<stdlib.h>

# include"token.h"
# include"config.h"

void print_token(Token token){
	printf("%.*s\n", (int)token.len, token.start);
}

const char*match_right_paren(const char*p){
	const char*s;
	int depth=1;
	if(!p || *p!='('){
		return NULL;
	}
	for(s=p+1; *s && s-p<=MAX_FACT_LEN; ++s){
        if(*s=='('){
			++depth;
		}else if(*s==')'){
			--depth;
			if(depth==0)
				return s;
		}
	}
	return NULL;
}

const char*strstr_with_skipping_brackets(
										 const char*src, const char*sub){
	const char*p, *bracket;
	p=src;
	do{
		bracket=strstr(p, "(");
		p=strstr(p, sub);
		if(!p){
			return NULL;
		}
		if(!bracket || bracket>=p){
			break;
		}
		bracket=match_right_paren(bracket);
		if(!bracket){
			return NULL;
		}
		p=bracket+1;
	}while(p-src<=MAX_FACT_LEN);
    if(p-src>MAX_FACT_LEN){
        return NULL;
    }
	return p;
}

void remove_outer_brackets(Token*token){
	const char*right_bracket;
	do{
		if(*(token->start)!='('){
			return;
		}
		right_bracket=match_right_paren(token->start);
		if(right_bracket-token->start+1-token->len==0){
			token->start++;
			token->len-=2;
		}else{
			return;
		}
	}while(1);
}

int token_to_tokens(
					   Token token, const char*wall, Tokens*out){
	const char*temp1, *temp2;
	int i;
	Token*tokens=out->tokens;
	out->cnt=0;
	temp1=token.start;
	i=0;
	if(token.len==0){
		out->cnt=0;
		return 1;
	}
	do{
		if(i>=MAX_TOKENS){
			return 0;
		}
		tokens[i].start=temp1;
		temp2=
		strstr_with_skipping_brackets(temp1, wall);
		if(!temp2 || (temp2-temp1)>=token.len){
			tokens[i].len=token.len-(temp1-token.start);
			remove_outer_brackets(tokens+i);
			i++;
			break;
		}
		tokens[i].len=temp2-temp1;
		remove_outer_brackets(tokens+i);
		temp1=temp2+strlen(wall);
		i++;
	}while(1);
	out->cnt=i;
	return 1;
}

void token_to_str(Token*token, char*out){
	memcpy(out, token->start, token->len);
	out[token->len]=0;
}
