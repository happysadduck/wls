#include <string.h>

#include "config.h"
#include "show.h"
#include "translator.h"

int parse_keyword(const char *keyword_cmd) {
    if (strcmp(keyword_cmd, IMPORT_KEY) == 0) {
        if (CURR_SYSTEM.current_keyword != 'i' &&
            CURR_SYSTEM.current_keyword != 'g') {
            /*you can only import/given at the beginning of a proof*/
            raise_error("# [ERROR]: " MIX_IGPS);
            return 0;
        }
        CURR_SYSTEM.current_keyword = 'i';
        return 0;
    }
    if (strcmp(keyword_cmd, GIVEN_KEY) == 0) {
        if (CURR_SYSTEM.current_context) {
            /*you can only import/given at the beginning of a proof*/
            raise_error("# [ERROR]: " MIX_IGPS);
            return 0;
        }
        CURR_SYSTEM.current_keyword = 'g';
        return 0;
    }
    if (strcmp(keyword_cmd, PROVE_KEY) == 0) {
        CURR_SYSTEM.current_keyword = 'p';
        CURR_SYSTEM.prove_try_cnt++;
        return 0;
    }
    if (strcmp(keyword_cmd, SUPPOSE_KEY) == 0) {
        CURR_SYSTEM.current_keyword = 's';
        CURR_SYSTEM.suppose_try_cnt++;
        return 0;
    }
    if (strcmp(keyword_cmd, DEBUG_KEY) == 0) {
        debug_screen();
        return 0;
    }
    if (strcmp(keyword_cmd, QUIT_KEY) == 0)
        return 1;
    if (strcmp(keyword_cmd, SURRENDER_KEY) == 0) {
        if (CURR_SYSTEM.current_context)
            close_context(0);
        return 0;
    }
    if (strcmp(keyword_cmd, EMPTY_KEY) == 0) {
        CURR_SYSTEM.current_keyword = 'e';
        return 0;
    }
    /*no such keyword*/
    raise_error("# [ERROR]: " KWORD_NFOUND);
    return 0;
}
