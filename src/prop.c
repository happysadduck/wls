#include <stdio.h>
#include <string.h>

#include "config.h"
#include "match.h"
#include "show.h"
#include "translator.h"

#define ANY_PROP_WALL "(" ANY_PROP_SIGN ":"
#define EXIST_PROP_WALL "(" EXIST_PROP_SIGN ":"

/**
 * @param prop_def_cmd prop_t 定义式
 * @param prop_mem     必须在 prop_def_cmd 的'\0' 结束符后, 并对齐四字节.
 * @param top 可操作内存的顶端 (包含)
 * @return 经过此操作后, 用掉的内存的顶端 (包含), 若空间不足返回 NULL,
 * 若解析失败返回 prop_mem.
 */
char *parse_prop_def(const char *prop_def_cmd, char *prop_mem, char *top) {
    prop_t *prop = (prop_t *)prop_mem;
    token_t name, paras, conds, concs;
    const char *temp1, *temp2;
    cnt_t legal_token_cnt;
    char *off = prop_mem + sizeof(prop_t);
    if (off > top)
        return NULL;
    temp1 = strstr_with_skipping_brackets(prop_def_cmd, ANY_PROP_WALL);
    temp2 = strstr_with_skipping_brackets(prop_def_cmd, EXIST_PROP_WALL);
    if (temp1 && temp2) {
        if (temp1 < temp2)
            prop->type = 'u';
        else {
            prop->type = 'e';
            temp1 = temp2;
        }
    } else if (temp1)
        prop->type = 'u';
    else if (temp2) {
        prop->type = 'e';
        temp1 = temp2;
    } else {
        /*not legal prop definition structure*/
        raise_error("# [ERROR]: " NLEG_DEF_CMD);
        return prop_mem;
    }
    name.start = OFF(prop_def_cmd);
    name.len = temp1 - prop_def_cmd;
    prop->core.start = OFF(temp1);
    temp1 = temp2 = NULL;
    paras.start = name.start +
                  ((prop->type == 'u') ? strlen(ANY_PROP_WALL)
                                       : strlen(EXIST_PROP_WALL)) +
                  name.len;
    conds.start = OFF(strstr_with_skipping_brackets(PTR(paras.start), ")"));
    if (!conds.start) {
        /*not legal prop definition structure*/
        raise_error("# [ERROR]: " NLEG_DEF_CMD);
        return prop_mem;
    }
    conds.start += strlen(")");
    paras.len = (conds.start - paras.start) - strlen(")");
    concs.start = OFF(strstr_with_skipping_brackets(PTR(conds.start), ":"));
    temp1 = PTR(concs.start);
    if (!temp1) {
        /*not legal prop definition structure*/
        raise_error("# [ERROR]: " NLEG_DEF_CMD);
        return prop_mem;
    }
    concs.start += strlen(":");
    conds.len = (concs.start - conds.start) - strlen(":");
    concs.len = strlen(temp1) - strlen(":");
    prop->core.len = concs.start - name.start - name.len + concs.len;
    remove_outer_brackets(&name);
    remove_outer_brackets(&paras);
    remove_outer_brackets(&conds);
    remove_outer_brackets(&concs);

    prop->paras.tokens = OFF(off);
    if (!token_to_tokens(paras, ",", &(prop->paras),
                         (top - off + 1) / sizeof(token_t)))
        return NULL;
    off += prop->paras.cnt * sizeof(token_t);
    if (off > top)
        return NULL;
    prop->conds.tokens = OFF(off);
    if (!token_to_tokens(conds, ",", &(prop->conds),
                         (top - off + 1) / sizeof(token_t)))
        return NULL;
    off += prop->conds.cnt * sizeof(token_t);
    if (off > top)
        return NULL;
    prop->concs.tokens = OFF(off);
    if (!token_to_tokens(concs, ",", &(prop->concs),
                         (top - off + 1) / sizeof(token_t)))
        return NULL;
    off += prop->concs.cnt * sizeof(token_t);
    if (off > top)
        return NULL;
    prop->name = name;
    prop->prop_def = OFF(prop_def_cmd);
    return off;
}

/**
 * @param prop_inv_cmd prop_t 调用命令, 没有要求
 * @param top          可写内存的顶端 (包含)
 * @return 写入内存的顶端 (不包含)
 */
char *parse_prop_inv(char *prop_inv_cmd, char *cmd_end, char *top) {
    char *end_of_name =
        (char *)strstr_with_skipping_brackets(prop_inv_cmd, "(");
    char *end_of_cmd = cmd_end;
    prop_t *prop;
    token_t name_token;
    token_t new_params_uncut;
    tokens_t new_params_raw;
    tokens_t new_params;
    cnt_t i;
    char removed;
    /*此处将 off 放置于命令结束符的后一个字节*/
    char *off = end_of_cmd + 1;

    /*对齐 4 字节*/
    ALI(off, 4);
    if (off > top)
        return NULL;
    if (!end_of_name) {
        /*not legal prop invocation structure*/
        raise_error("# [ERROR]: " NLEG_INV_CMD);
        return end_of_cmd;
    }
    name_token.start = OFF(prop_inv_cmd);
    name_token.len = end_of_name - prop_inv_cmd;
    /*此处不去括号, 因为实验发现: 括号是识别参数区开始的唯一凭证,
     * 所以无法引用带括号的命题, 即使可以这么定义.*/
    removed = token_cut(name_token);
    prop = tower_find(&CURR_SYSTEM.prop_table, PTR(name_token.start));
    if (!prop) {
        /*no such proposition*/
        raise_error("# [ERROR]: " PROP_NFOUND);
        return end_of_cmd;
    }
    token_restore(name_token, removed);
    new_params_uncut.start = OFF(end_of_name);
    new_params_uncut.len = end_of_cmd - end_of_name;
    remove_outer_brackets(&new_params_uncut);

    /*提取到的粗 tokens 数组放置在命令字符串后对齐 4 字节的位置*/
    new_params_raw.tokens = OFF(off);
    if (!token_to_tokens(new_params_uncut, ",", &new_params_raw,
                         (top - off + 1) / sizeof(token_t)))
        return NULL;
    if (prop->paras.cnt != new_params_raw.cnt) {
        /*too few or too many parameters*/
        raise_error("# [ERROR]: " WPAR_CNT);
        return end_of_cmd;
    }
    /*对于 "e" 型 prop, 处理过的 tokens 放在 raw tokens 数组后.*/
    /*此处总是如此操作, 因此后面处理 "u" 型需要调整回来*/
    /*off 此处位于处理过的 tokens 数组之后的位置*/
    new_params.tokens = OFF(off) + new_params_raw.cnt * sizeof(token_t);
    off += new_params_raw.cnt * 2 * sizeof(token_t);
    if (off > top)
        return NULL;
    if (prop->type == 'e') {
        for (i = 0; i < new_params_raw.cnt; i++) {
            token_t *param_old = (token_t *)PTR(new_params_raw.tokens) + i;
            token_t *param_new = (token_t *)PTR(new_params.tokens) + i;
            tower_node_t *mem_node = pool_alloc(&CURR_SYSTEM.pool_for_towers);
            char removed;
            token_cut(*param_old); /*此处切割命题调用式, 不影响*/
            param_new->start = OFF(off);
            off = construct_param_name(&CURR_SYSTEM.cons_para_cnt, off, top);
            if (!off)
                return NULL;
            param_new->len = OFF(off) - param_new->start;
            off++;
            if (off > top)
                return NULL;
            if (!tower_insert(&CURR_SYSTEM.used_paras, mem_node,
                              PTR(param_old->start), PTR(param_new->start))) {
                /*repieition define*/
                raise_error("# [ERROR]: " REDEF_PAR);
                return end_of_cmd;
            }
        }
        new_params.cnt = new_params_raw.cnt;
    } else {
        char *mem_cond;
        off = PTR(new_params.tokens);
        /*由于 prop 为 "u" 型, 实际上没有 "处理过的 tokens 数组", off 移动回 raw
         * tokens 数组后的区域*/
        mem_cond = off;
        /*"u" 型 prop 还需要一个检查 cond 的步骤. 借用 raw tokens 数组后的区域.
         */
        new_params = new_params_raw;
        for (i = 0; i < prop->conds.cnt; i++) {
            token_t *cond = (token_t *)PTR(prop->conds.tokens) + i;
            off =
                replace_braces(cond, &prop->paras, &new_params, mem_cond, top);
            if (!off)
                return NULL;
            if (!is_in_tower(&CURR_SYSTEM.fact_set, mem_cond)) {
                /*lack necessary premise*/
                raise_error("# [ERROR]: " LACK_PRM);
                return end_of_cmd;
            }
        }
        off = mem_cond;
        /*操作完成后, off 应该始终位于 tokens 数组末尾*/
    }
    /*off 应该停留在正确区域*/
    for (i = 0; i < prop->concs.cnt; i++) {
        token_t *conc = (token_t *)PTR(prop->concs.tokens) + i;
        tower_node_t *mem_node = pool_alloc(&CURR_SYSTEM.pool_for_towers);
        char *mem_conc = off;
        off = replace_braces(conc, &prop->paras, &new_params, off, top);
        /*off 向后移动, 从而为每一个 conc token 腾出空间*/
        if (!off)
            return NULL;
        off++;
        /*off 再移动一格, 因为 replace braces 返回字符串 '\0' 的位置,
         * 需要额外移动一格*/
        if (off > top)
            return NULL;
        tower_insert(&CURR_SYSTEM.fact_set, mem_node, mem_conc, NULL);
    }
    return off;
}
