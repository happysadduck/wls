# include<stdio.h>

# include"input.h"
# include"core.h"
# include"translator.h"
# include"alarm.h"

static int get_cmd(FILE*f, char*out){
	int ch;
	int prev=0;
	char*p=out;
	while((ch=getc(f))!=EOF){
		if(ch==' '){
			continue;
		}
		if(ch=='#'){
            if(prev=='.' || prev==':'){
                --p;
                *p=0;
            }
			goto eat_line;
		}
		if(ch=='\n'){
			if(prev=='.'){
				--p;
				*p=0;
				return 1;
			}
			if(prev==':'){
				--p;
				*p=0;
				return -1;
			}
			continue;
		}
		*p++=(char)ch;
		prev=ch;
        if(p-out>=MAX_FACT_LEN){
            while((ch=getc(f))!=EOF && ch!='\n');
            printf("[SEVERER ERROR]: too long command!\n");
            return 0;
        }
	}
	*p=0;
	return 0;
	
	eat_line:
	while((ch=getc(f))!=EOF && ch!='\n');
	*p=0;
	return(prev==':')? -1: 1;
}

int read_and_run(FILE*f, WLS*system){
    char*data=pool_alloc(system->pool_for_props);
    char*cmd=data+sizeof(Proposition);
    if(!data){
        printf("[SEVERER ERROR]: prop pool used up\n");
        return 0;
    }
    system->line_cnt++;
    switch(get_cmd(f, cmd)){
    case -1:{
        int key_word_parse_result=
        parse_key_words(system, cmd);
        if(!key_word_parse_result){
            RAISE_ERROR("no such keyword", system);
            return 1;
        }if(key_word_parse_result==-1)
            return 0;
        return 1;
    }
    case 1: 
        if(!cmd || !*cmd) return 1;
        if(process_cmd(system, cmd)==-1) return 0;
        return 1;
    default: return 0;
    }
}
