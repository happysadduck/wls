# include<stdio.h>

# include"match.h"

void batch_replace(
				   Tokens*params,
				   Tokens*new_params,
				   const char*src,
				   char*out){
	int pn=params->cnt;
	const char*s=src;
	char*d=out;
	if(!params || !new_params || !src || !out){
		strcpy(out, src);
		return;
	}
	if(!pn || pn!=new_params->cnt){
		strcpy(out, src);
		return;
	}
	while(*s){
		int matched=-1;
		int i;
		for(i=0; i<pn; i++){
			const Token*pat=&params->tokens[i];
			if(memcmp(s, pat->start, pat->len)==0){
				matched=i;
				break;
			}
		}
		if(matched>=0){
			const Token*rep=&new_params->tokens[matched];
			memcpy(d, rep->start, rep->len);
			d+=rep->len;
			s+=params->tokens[matched].len;
		}else{
			*d++=*s++;
		}
	}
	*d=0;
}

void carpet_replace(
                    HashTower*t, 
                    Token*src,
                    Token*out){
    int len=src->len;
    char*p=(char*)src->start;
    char*end=p+len;
    char*w=(char*)out->start;
    while(p<end){
        int max_len=end-p;
        int match_len=0;
        const char*val=NULL;
        int l;
        for(l=1; l<=max_len; l++){
            char old=p[l];
            p[l]=0;
            val=tower_find(t, p);
            p[l]=old;
            if(val){
                match_len=l;
                break;
            }
        }
        if(match_len){
            int val_len=strlen(val);
            memcpy(w, val, val_len);
            w+=val_len;
            p+=match_len;
        }else  *w++=*p++;
    }
    *w=0;
    out->len=w-out->start;
}
