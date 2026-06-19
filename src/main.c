#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "core.h"
#include "input.h"
#include "show.h"
#include "translator.h"

char *MEM_BASE;
char *TOP;
WLS CURR_SYSTEM;

int main(int argc, char **argv) {
    FILE *fp;
    if (argc > 1)
        fp = fopen(argv[1], "r");
    else {
        fp = stdin;
        printf(HELLO_WORDS);
        printf("\n");
    }
    MEM_BASE = malloc(TOTAL_MEM);
    prepare_system(fp != stdin);
    while (1) {
        char *cmd = get_cmd_mem();
        int end_with_colon = 0;
        int is_terminal = 0;
        char *cmd_end;
        cnt_t lines_span;
        TOP = MEM_BASE + TOTAL_MEM - 1;
        cmd_end =
            get_cmd(fp, cmd, TOP, &end_with_colon, &is_terminal, &lines_span);
        CURR_SYSTEM.line_cnt += lines_span;
        if (is_terminal)
            break;
        if (!cmd_end) {
            /*severe error: memory used up*/
            printf("[SEVERE]: " NSTACK "\n");
            break;
        }
        if (end_with_colon) {
            if (parse_keyword(cmd))
                break;
        } else if (process_cmd(cmd, cmd_end) == -100) {
            /*severe error: memory used up*/
            printf("[SEVERE]: " NSTACK "\n");
            break;
        } else if (process_cmd(cmd, cmd_end) == -200) {
            /*severe error: pool used up*/
            printf("[SEVERE]: " NPOOL "\n");
            break;
        }
    }
    if (fp != stdin)
        print_wls_report();
    free(MEM_BASE);
    fclose(fp);
    return 0;
}
