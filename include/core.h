#ifndef CORE_H
#define CORE_H

#include "hash.h"
#include "pool.h"
#include "token.h"

typedef struct {
    moff_t parent;
    moff_t value;
    moff_t reference; /*用于证明 u 型 prop 时, 对去除了大括号的 concs 进行缓存.
                         从而避免每个循环都要解析一遍. */
    /*存储方式: 从此位置开始, 连续 prop->concs.cnt 个字符串, 首尾紧密连接*/
    char behavior;
} context_t;

typedef struct {
    token_t name;
    token_t core;
    moff_t prop_def;
    char type;
    tokens_t paras;
    tokens_t conds;
    tokens_t concs;
} prop_t;

typedef struct {
    cnt_t cons_para_cnt;
    cnt_t line_cnt;
    char current_keyword;
    cnt_t prove_try_cnt;
    cnt_t prove_achieve_cnt;
    cnt_t suppose_try_cnt;
    cnt_t suppose_achieve_cnt;
    cnt_t error_cnt;
    cnt_t depth;
    context_t *current_context;
    hash_tower_t fact_set;
    hash_tower_t used_paras;
    hash_tower_t prop_table;
    layer_manager_t controller;
    pool_t pool_for_sets;
    pool_t pool_for_towers;
    char *stack_start_front;
    char *stack_start_behind;
    char *stack_top;
    int use_file;
} WLS;

extern WLS CURR_SYSTEM;

void prepare_system(int use_file);

int close_context(int success);

char *get_cmd_mem(void);
int process_cmd(char *cmd, char *end);

#endif
