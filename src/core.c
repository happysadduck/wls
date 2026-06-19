#include "core.h"
#include "config.h"

void prepare_system(int use_file) {
    char *stack_top = MEM_BASE;
    layer_t *first_layer;
    set_node_t *first_lv_set_node;
    CURR_SYSTEM.cons_para_cnt = 0;
    CURR_SYSTEM.line_cnt = 0;
    CURR_SYSTEM.prove_try_cnt = 0;
    CURR_SYSTEM.prove_achieve_cnt = 0;
    CURR_SYSTEM.suppose_try_cnt = 0;
    CURR_SYSTEM.suppose_achieve_cnt = 0;
    CURR_SYSTEM.error_cnt = 0;
    CURR_SYSTEM.depth = 0;
    CURR_SYSTEM.current_keyword = 'i';
    stack_top = prepare_layer_manager(&CURR_SYSTEM.controller, stack_top,
                                      LAYER_BUC_CNT);
    stack_top++;
    stack_top = prepare_tower(&CURR_SYSTEM.fact_set, stack_top, FACT_BUC_CNT,
                              &CURR_SYSTEM.controller);
    stack_top++;
    stack_top = prepare_tower(&CURR_SYSTEM.used_paras, stack_top, PARA_BUC_CNT,
                              &CURR_SYSTEM.controller);
    stack_top++;
    stack_top = prepare_tower(&CURR_SYSTEM.prop_table, stack_top, PROP_BUC_CNT,
                              &CURR_SYSTEM.controller);
    stack_top++;
    ALI(stack_top, 8);

    stack_top = prepare_pool(&CURR_SYSTEM.pool_for_sets, stack_top,
                             sizeof(set_node_t), MAX_SNODES);
    stack_top++;
    stack_top = prepare_pool(&CURR_SYSTEM.pool_for_towers, stack_top,
                             sizeof(tower_node_t), MAX_TNODES);
    stack_top++;

    CURR_SYSTEM.stack_start_front = stack_top;
    CURR_SYSTEM.stack_start_behind = stack_top;
    *(char **)stack_top = NULL;
    stack_top += sizeof(char *);
    *(char **)stack_top = NULL;
    stack_top += sizeof(char *);

    first_layer = (layer_t *)stack_top;
    first_layer->lv = 0;
    first_layer->prev = OFF(NULL);
    CURR_SYSTEM.controller.curr_layer = first_layer;
    first_lv_set_node = pool_alloc(&CURR_SYSTEM.pool_for_sets);
    set_insert(&CURR_SYSTEM.controller.using_lvs_set, first_lv_set_node, 0);
    stack_top += sizeof(layer_t);

    CURR_SYSTEM.stack_top = stack_top;

    CURR_SYSTEM.use_file = use_file;
}

char *get_cmd_mem() { return CURR_SYSTEM.stack_top; }
