#include <stdio.h>
#include <string.h>

#include "config.h"
#include "core.h"
#include "match.h"
#include "show.h"
#include "translator.h"

/**
 * @brief 添加 prop 到系统中, 要求 CURR_SYSTEM 栈顶恰好位于此 prop 的定义式顶端
 * '\0' 后的一个字节处.
 * @return 内存不足的种类.
 */
static int add_prop(prop_t *prop) {
    char *off = CURR_SYSTEM.stack_top;
    tower_node_t *node_mem_prop, *node_mem_fact;
    char *name_mem = off;
    node_mem_prop = pool_alloc(&CURR_SYSTEM.pool_for_towers);
    node_mem_fact = pool_alloc(&CURR_SYSTEM.pool_for_towers);
    if (!node_mem_prop || !node_mem_fact)
        return -200;
    off = token_to_str(prop->name, name_mem, TOP);
    if (!off)
        return -100;
    tower_insert(&CURR_SYSTEM.prop_table, node_mem_prop, name_mem, prop);
    tower_insert(&CURR_SYSTEM.fact_set, node_mem_fact, PTR(prop->core.start),
                 NULL);
    if (prop->type == 'e') {
        cnt_t i;
        for (i = 0; i < prop->paras.cnt; i++) {
            /*将证明的 e 型 prop 的参数取消含义, 以为更多的构造腾出空间.*/
            token_t *para = (token_t *)PTR(prop->paras.tokens) + i;
            char removed = token_cut(*para);
            char *hanged_node_param =
                tower_delete(&CURR_SYSTEM.used_paras, PTR(para->start));
            token_restore(*para, removed);
            if (hanged_node_param)
                pool_return(&CURR_SYSTEM.pool_for_towers, hanged_node_param);
        }
    }
    CURR_SYSTEM.stack_top = off + 1;
    return 0;
}

int close_context(int success) {
    set_node_t *hanged_set_node;
    char *off;
    context_t backup = *CURR_SYSTEM.current_context;
    tower_cut_layer(&CURR_SYSTEM.controller, &hanged_set_node);
    pool_return(&CURR_SYSTEM.pool_for_sets, hanged_set_node);
    CURR_SYSTEM.current_context = PTR(CURR_SYSTEM.current_context->parent);
    CURR_SYSTEM.depth--;

    if (!success) {
        char *temp = CURR_SYSTEM.stack_start_behind;
        off = CURR_SYSTEM.stack_start_front;
        CURR_SYSTEM.stack_start_front = *(char **)temp;
        CURR_SYSTEM.stack_start_behind = *((char **)temp + 1);
        CURR_SYSTEM.stack_top = off;
        return 0;
    }
    off = CURR_SYSTEM.stack_start_behind;
    CURR_SYSTEM.stack_start_front = *(char **)off;
    CURR_SYSTEM.stack_start_behind = *((char **)off + 1);
    CURR_SYSTEM.stack_top = off;
    if (backup.behavior == 'p') {
        prop_t *proved = PTR(backup.value);
        int add_prop_result = add_prop(proved);
        if (add_prop_result)
            return add_prop_result;
        /*successfully proved: prop name*/
        printf("# [INFO]: %s: %.*s", SUC_PROV_PRE, proved->name.len,
               (char *)PTR(proved->name.start));
        if (CURR_SYSTEM.use_file)
            printf(" (line: %u)", CURR_SYSTEM.line_cnt);
        printf("\n");
        CURR_SYSTEM.prove_achieve_cnt++;
    } else {
        char *suppose = PTR(backup.value);
        cnt_t len = strlen(suppose);
        /*先移动足以容纳 nagtive suppose的空间, 写入一份 nagtive suppose*/
        /*移动应该是合法的, 因为 suppose 字符串应当位于栈区域的顶端.*/
        char *nagtive_suppose =
            suppose + len + 4; /*!() 三字节长, '\0' 一字节长*/
        /*会自己检查是否超长, 无需确保 nagtive suppose 在 TOP 下*/
        if (!construct_nagtive_suppose(suppose, nagtive_suppose, TOP))
            return -100;
        tower_node_t *mem_tower_node = pool_alloc(&CURR_SYSTEM.pool_for_towers);
        /*将 nagtive suppose移动回原 suppose 存放的区域. */
        memcpy(suppose, nagtive_suppose, len + 4);
        /*将栈顶放在移动回原位的 nagtive suppose 顶端后*/
        CURR_SYSTEM.stack_top = nagtive_suppose;
        tower_insert(&CURR_SYSTEM.fact_set, mem_tower_node, suppose, NULL);
        CURR_SYSTEM.suppose_achieve_cnt++;
        /*successfully proved: nagtive suppose*/
        printf("# [INFO]: %s: %s", SUC_PROV_PRE, suppose);
        if (CURR_SYSTEM.use_file)
            printf(" (line: %u)", CURR_SYSTEM.line_cnt);
        printf("\n");
    }
    return 0;
}

/**
 * @brief 添加层和扩建栈区. 要求CURR_SYSTEM.stack_top 位于to_be_prove_prop +
 * 其所有尾随token_t之后. 或者位于suppose内容字符串的'\0'字符后一字节. 不要对齐.
 * 操作后, stack_top 会移动到新的栈区的顶端. 甚至对齐指针字节长
 * @param start_front 必然是 def_cmd 或者 suppose_fact 命令的起点.
 */
static int add_context(void *value, char behavior, char *start_front) {
    context_t *new_context;
    set_node_t *layer_node_mem = pool_alloc(&CURR_SYSTEM.pool_for_sets);
    char *off = CURR_SYSTEM.stack_top;
    char *start_of_stack_level;
    if (!layer_node_mem)
        return -200;
    ALI(off, 8);
    /*栈区起点*/
    start_of_stack_level = off;
    if (off + 2 * sizeof(char *) > TOP)
        return -100;
    /*每个栈区的开头必然是指向上一个背景的前起和后起的指针*/
    *(char **)off = CURR_SYSTEM.stack_start_front;
    off += sizeof(char *);
    *(char **)off = CURR_SYSTEM.stack_start_behind;
    off += sizeof(char *);
    CURR_SYSTEM.stack_start_front = start_front;
    CURR_SYSTEM.stack_start_behind = start_of_stack_level;

    new_context = (context_t *)(off);
    new_context->behavior = behavior;
    new_context->value = OFF(value);
    new_context->parent = OFF(CURR_SYSTEM.current_context);
    CURR_SYSTEM.current_context = new_context;
    off += sizeof(context_t);

    tower_add_layer(&CURR_SYSTEM.controller, off, layer_node_mem);
    off += sizeof(layer_t);

    CURR_SYSTEM.stack_top = off;
    CURR_SYSTEM.depth++;

    return 0;
}

int process_cmd(char *cmd, char *end) {
    context_t *old_context;
    if (!*cmd)
        return 0;
    switch (CURR_SYSTEM.current_keyword) {
    case 'e': {
        char *off = parse_prop_inv(cmd, end, TOP);
        if (off == end) {
            CURR_SYSTEM.stack_top = cmd;
            return 0;
        }
        if (!off)
            return -100;
        CURR_SYSTEM.stack_top = off + 1;
        break;
    }
    case 'i': {
        prop_t *new_prop = (prop_t *)(end + 1);
        char *off;
        int add_prop_result;
        ALI(new_prop, 4);
        off = parse_prop_def(cmd, (char *)new_prop, TOP);
        /*解析失败*/
        if (off == (char *)(new_prop))
            return 0;
        CURR_SYSTEM.stack_top = off + 1;
        add_prop_result = add_prop(new_prop);
        if (add_prop_result)
            return add_prop_result;
        break;
    }
    case 'g': {
        tower_node_t *node_mem = pool_alloc(&CURR_SYSTEM.pool_for_towers);
        if (!node_mem)
            return -200;
        tower_insert(&CURR_SYSTEM.fact_set, node_mem, cmd, NULL);
        CURR_SYSTEM.stack_top = end + 1;
        break;
    }
    case 'p': {
        prop_t *to_be_prove = (prop_t *)(end + 1);
        /*根据 parse_prop_def 的要求, 将 prop_t 空间放在命令结束符的末尾*/
        context_t *new_context;
        cnt_t i;
        char *off;
        char removed;
        int flag;
        int add_prop_result;
        set_node_t *layer_node_mem;
        int add_context_result;
        ALI(to_be_prove, 4);
        /*对齐 prop_t 所需空间*/
        CURR_SYSTEM.current_keyword = 'e';
        off = parse_prop_def(cmd, (char *)to_be_prove, TOP);
        if (!off)
            return -100;
        /*解析失败*/
        if (off == (char *)(to_be_prove))
            return 0;
        /*此处检查 prop_core 是否直接位于 fact set 中*/
        removed = token_cut(to_be_prove->core);
        flag = is_in_tower(&CURR_SYSTEM.fact_set, PTR(to_be_prove->core.start));
        token_restore(to_be_prove->core, removed);
        CURR_SYSTEM.stack_top = ++off;
        /*目前 off 位于 prop 占据空间的顶端*/
        /*栈顶也位于此处, 接下来如果证明成功, 对于 e 型其不再移动 (移交
         * add_prop), 否则应该移动回 cmd. */
        /*而对于 u 型, 如果直接证明完毕, 则]直接移交 add_prop. 否则, 开新
         * context 后, 需要立即占用几个用于 concs 字符串的空间*/
        if (flag) {
            /*如果在其中, 需要直接证明完毕*/
            add_prop_result = add_prop(to_be_prove);
            if (add_prop_result)
                return add_prop_result;
            /*successfully proved: prop name*/
            printf("# [INFO]: %s: %.*s", SUC_PROV_PRE, to_be_prove->name.len,
                   (char *)PTR(to_be_prove->name.start));
            if (CURR_SYSTEM.use_file)
                printf(" (line: %u)", CURR_SYSTEM.line_cnt);
            printf("\n");
            CURR_SYSTEM.prove_achieve_cnt++;
            break;
        }
        /*e 型 prop 证明相对简单, 只需要将每一个 para 的构造值带回 concs
         * 检查是否均存在即可*/
        if (to_be_prove->type == 'e') {
            tokens_t construct_params;
            /*对齐 off 以准备准备存储construct params 的 token 数组*/
            ALI(off, 4);
            if (off > TOP)
                return -100;
            flag = 0;
            construct_params.tokens = OFF(off);
            construct_params.cnt = to_be_prove->paras.cnt;
            if (off + to_be_prove->paras.cnt * sizeof(token_t) > TOP)
                return -100;
            /*写construct params 的 token 数组*/
            for (i = 0; i < to_be_prove->paras.cnt; i++) {
                token_t *token = (token_t *)(off);
                token_t curr_token =
                    ((token_t *)PTR(to_be_prove->paras.tokens))[i];
                char *corelated_para_name;
                /*此处临时切割construct params, 用于读取对应的构造名*/
                char removed = token_cut(curr_token);
                corelated_para_name =
                    tower_find(&CURR_SYSTEM.used_paras, PTR(curr_token.start));
                token_restore(curr_token, removed);
                if (!corelated_para_name) {
                    /*construction prove failed: not all target params have been
                     * constructed*/
                    raise_error("# [ERROR]: " LACK_CON_PAR);
                    CURR_SYSTEM.stack_top = cmd;
                    /*flag 用于跳出两层循环, 反正正好空着不用, 遂不使用 goto*/
                    flag = 1;
                    break;
                }
                token->start = OFF(corelated_para_name);
                token->len = strlen(corelated_para_name);
                off += sizeof(token_t);
            }
            /*此处 off 应该位于construct params 的 token
             * 数组的顶端的上一个字节*/
            if (flag) {
                /*证明失败: 有的 param 没有被构造*/
                CURR_SYSTEM.stack_top = cmd;
                break;
            }
            /*然后开始写入被construct params 替换的 concs, 每个就是一次性检查,
             * 所以可以覆写*/
            for (i = 0; i < to_be_prove->concs.cnt; i++) {
                char *buf_start = off;
                off = replace_braces(
                    (token_t *)PTR(to_be_prove->concs.tokens) + i,
                    &to_be_prove->paras, &construct_params, buf_start, TOP);
                if (!off)
                    return -100;
                if (!is_in_tower(&CURR_SYSTEM.fact_set, buf_start)) {
                    /*not all features of params are proved*/
                    raise_error("# [ERROR]: " LACK_CON_FET);
                    CURR_SYSTEM.stack_top = cmd;
                    flag = 1;
                    break;
                }
            }
            if (flag) {
                /*证明失败: 并没有所有的条件达成*/
                CURR_SYSTEM.stack_top = cmd;
                break;
            }
            /*证明成功*/
            add_prop_result = add_prop(to_be_prove);
            if (add_prop_result)
                return add_prop_result;
            /*successfully proved: prop name*/
            printf("# [INFO]: %s: %.*s", SUC_PROV_PRE, to_be_prove->name.len,
                   (char *)PTR(to_be_prove->name.start));
            if (CURR_SYSTEM.use_file)
                printf(" (line: %u)", CURR_SYSTEM.line_cnt);
            printf("\n");
            CURR_SYSTEM.prove_achieve_cnt++;
            break;
        }
        /*此处只剩下 u 型 prop 的操作, 由于没有直接证得, 需要开新的 context*/
        /*目前 off 正确位于 prop_t 结构体末尾上一字节*/
        add_context_result = add_context(to_be_prove, 'p', cmd);
        off = CURR_SYSTEM.stack_top;
        /*现在 off 在新的栈顶, 接下来在新栈区中写入下面的辅助内容*/
        if (add_context_result)
            return add_context_result;
        /*此处连续写入多个 conds 字符串, 是原 conds 以原参数替代得到的,
         * 这是为了去掉其中的大括号*/
        for (i = 0; i < to_be_prove->conds.cnt; i++) {
            token_t *cond = (token_t *)PTR(to_be_prove->conds.tokens) + i;
            char *new_cond = off;
            tower_node_t *mem_tower_node =
                pool_alloc(&CURR_SYSTEM.pool_for_towers);
            if (!mem_tower_node)
                return -200;
            off = replace_braces(cond, &to_be_prove->paras, &to_be_prove->paras,
                                 new_cond, TOP);
            /*移动到替换得到的 cond 末尾*/
            if (!off)
                return -100;
            off++; /*额外偏移一字节, 彻底走到 cond 之后*/
            tower_insert(&CURR_SYSTEM.fact_set, mem_tower_node, new_cond, NULL);
            /*将这些新 cond 插入 fact set*/
        }
        /*此时 off 应该位于所有 conds 的顶端上*/
        /*此时还需缓存每次检查 context 结束所需的结论参考值. 这些字符串不加入到
         * fact set 中, 但会被不断引用*/
        /*此时 off 所在位置就是这些参考值的起点*/
        CURR_SYSTEM.current_context->reference = OFF(off);
        for (i = 0; i < to_be_prove->concs.cnt; i++) {
            char *conc_mem = off;
            token_t *conc = (token_t *)PTR(to_be_prove->concs.tokens) + i;
            off = replace_braces(conc, &to_be_prove->paras, &to_be_prove->paras,
                                 conc_mem, TOP);
            if (!off)
                return -100;
            off++;
        }
        /*此时 off 位置正确, 位于缓存 concs 的顶端上*/
        CURR_SYSTEM.stack_top = off;
        break;
    }
    case 's': {
        context_t *new_context;
        set_node_t *layer_node_mem = pool_alloc(&CURR_SYSTEM.pool_for_sets);
        tower_node_t *mem_tower_node = pool_alloc(&CURR_SYSTEM.pool_for_towers);
        char *off = end + 1;
        int add_context_result;
        if (!(layer_node_mem && mem_tower_node))
            return -200;

        CURR_SYSTEM.stack_top = off;
        add_context_result = add_context(cmd, 's', cmd);
        if (add_context_result)
            return add_context_result;
        tower_insert(&CURR_SYSTEM.fact_set, mem_tower_node, cmd, NULL);
        CURR_SYSTEM.current_keyword = 'e';
    }
    default:
        break;
    }
    while (context_is_end()) {
        int close_context_result = close_context(1);
        if (close_context_result)
            return close_context_result;
    }
    return 0;
}
