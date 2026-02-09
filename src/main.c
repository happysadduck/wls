# include<stdio.h>
# include<string.h>

# include"core.h"
# include"translator.h"
# include"input.h"
# include"alarm.h"

# if USE_FILE
# define GET_FILE \
char file_path[101];\
FILE*f;\
scanf("%s", file_path);\
f=fopen(file_path, "r");
# define SRC_CLOSE fclose(f);
# define REPORT(system) \
printf("\n===================="\
"REPORT====================\n");\
printf("line: %d, error: %d, \n"\
"prove_try: %d, prove_achieve: %d, \n"\
"suppose_try: %d, suppose_achieve: %d\n", \
system->line_cnt, system->error_cnt, \
system->prove_try_cnt, system->prove_achieve_cnt, \
system->suppose_try_cnt, system->suppose_achieve_cnt)
# define EXTRA_ARG_ACT f, 
# else
# define GET_FILE
# define SRC_CLOSE
# define REPORT(system)
# define EXTRA_ARG_ACT
# endif

int main(void){
	WLS*system=create_system();
    int c;
	GET_FILE
	while(read_and_run(EXTRA_ARG_ACT system));
    system->line_cnt--;
    REPORT(system);
	destroy_system(system);
    SRC_CLOSE;
    printf("press Enter to exit...");
    while((c=getchar())!=EOF && c!='\n'){};
	return 0;
}
