#include <string.h>

#include "config.h"
#include "translator.h"

int context_is_end() {
    context_t *c = CURR_SYSTEM.current_context;
    if (!c)
        return 0;
    if (c->behavior == 'p') {
        prop_t *prop = PTR(c->value);
        cnt_t i;
        /*检验 prop core 字符串是否直接被证明出, 如果证明出, 直接成功*/
        char removed = token_cut(prop->core);
        int flag = is_in_tower(&CURR_SYSTEM.fact_set, PTR(prop->core.start));
        char *off = PTR(c->reference);
        token_restore(prop->core, removed);
        if (flag)
            return 1;
        /*否则看每一个结论是否证得*/
        for (i = 0; i < prop->concs.cnt; i++) {
            flag = !is_in_tower(&CURR_SYSTEM.fact_set, off);
            off += strlen(off);
            off++; /*移动到下一个字符串*/
            if (flag)
                return 0;
        }
        return 1;
    }
    if (c->behavior == 's') {
        if (is_in_tower(&CURR_SYSTEM.fact_set, CONFLICT_MESSAGE))
            return 1;
    }
    return 0;
}
