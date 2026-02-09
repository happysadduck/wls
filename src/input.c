# include<stdio.h>

# include"input.h"
# include"core.h"
# include"translator.h"
# include"alarm.h"

static int get_cmd_GENERIC();

# if USE_FILE
# define SRC_CH getc(fp)
# define EXTRA_ARG FILE*fp, 
# define GET_CMD(out) get_cmd_GENERIC(fp, out)
# else
# define DECL_FP
# define SRC_CH getchar()
# define EXTRA_ARG
# define GET_CMD(out) get_cmd_GENERIC(out)
# endif

int read_and_run(EXTRA_ARG WLS*system){
    char*data=pool_alloc(system->pool_for_props);
    char*cmd=data+sizeof(Proposition);
    if(!data){
        printf("[SEVERER ERROR]: prop pool used up\n");
        printf("press Enter to exit...");
        getchar();
        return 0;
    }
    system->line_cnt++;
    switch(GET_CMD(cmd)){
    case -1:
        if(!parse_key_words(system, cmd)){
            RAISE_ERROR("no such keyword", system);
            return 1;
        }
        return 1;
    case 1: 
        if(!cmd || !*cmd) return 1;
        if(strcmp(cmd, "quit")==0) return 0;
        if(process_cmd(system, cmd)==-1) return 0;
        return 1;
    default: return 0;
    }
}

static int get_cmd_GENERIC(EXTRA_ARG char*out){
	int ch;
	int prev=0;
	char*p=out;
	while((ch=SRC_CH)!=EOF){
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
            while((ch=SRC_CH)!=EOF && ch!='\n');
            printf("[SEVERE ERROR]: too long command!\n");
            printf("press Enter to exit...");
            getchar();
            return 0;
        }
	}
	*p=0;
	return 0;
	
	eat_line:
	while((ch=SRC_CH)!=EOF && ch!='\n');
	*p=0;
	return(prev==':')? -1: 1;
}
