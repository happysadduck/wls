# include<stdio.h>
# include<string.h>

# include"core.h"
# include"translator.h"
# include"input.h"
# include"alarm.h"

int main(int argc, char**argv){
	WLS*system=create_system();
    FILE*f;
    if(argc>1){
        f=fopen(argv[1], "r");
    }else{
        f=stdin;
    }
    printf("welcome to wolfarch v0.3\n");
	while(read_and_run(f, system));
    system->line_cnt--;
    if(argc>1){
        printf("\n===================="
        "REPORT====================\n");
        printf("line: %d, error: %d, \n"
        "prove_try: %d, prove_achieve: %d, \n"
        "suppose_try: %d, suppose_achieve: %d\n", 
        system->line_cnt, system->error_cnt, 
        system->prove_try_cnt, system->prove_achieve_cnt, 
        system->suppose_try_cnt, system->suppose_achieve_cnt);
    }
	destroy_system(system);
    fclose(f);
	return 0;
}
