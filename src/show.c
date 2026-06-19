#include <assert.h>
#include <stdio.h>

#include "config.h"
#include "show.h"

void print_token(token_t token) {
    char removed = token_cut(token);
    char *token_start = PTR(token.start);
    if (!token_start)
        return;
    printf("%s", token_start);
    token_restore(token, removed);
}

void print_tokens(tokens_t tokens) {
    cnt_t i;
    printf("[%u]: ", tokens.cnt);
    for (i = 0; i < tokens.cnt; i++) {
        token_t *token = (token_t *)PTR(tokens.tokens) + i;
        print_token(*token);
        printf("    ");
    }
    printf("\n");
}

void show_prop(prop_t *prop, char *indent_spaces) {
    cnt_t i;
    printf("%s", indent_spaces);
    print_token(prop->name);
    printf("\n");
    printf("%s", indent_spaces);
    printf("  paras");
    print_tokens(prop->paras);
    printf("%s", indent_spaces);
    printf("  conds");
    print_tokens(prop->conds);
    printf("%s", indent_spaces);
    printf("  concs");
    print_tokens(prop->concs);
}

void show_context() {
    context_t *c = CURR_SYSTEM.current_context;
    if (!c) {
        printf("no context\n\n");
        return;
    }
    if (c->behavior == 'p') {
        printf("proving:\n");
        show_prop(PTR(c->value), "  ");
    } else {
        printf("supposing:\n");
        printf("  ");
        printf("%s\n", (char *)PTR(c->value));
    }
    printf("\n");
}

void show_hash_tower(hash_tower_t *t) {
    cnt_t i;
    for (i = 0; i < t->bucket_cnt; i++) {
        tower_node_t *p = PTR(t->bucket[i]);
        while (p) {
            if (is_in_set(&t->manager->using_lvs_set, p->layer_create))
                printf("%s    ", (char *)PTR(p->key));
            p = PTR(p->next);
        }
    }
    printf("\n");
}

void show_memory_screen() {
    printf("pool set avail: %u\n",
           MAX_SNODES - CURR_SYSTEM.pool_for_sets.alloc_cnt);
    printf("pool tower avail: %u\n",
           MAX_TNODES - CURR_SYSTEM.pool_for_towers.alloc_cnt);
    printf("stack memory avail (B): %u\n",
           (unsigned int)(TOP - CURR_SYSTEM.stack_top));
}

void debug_screen() {
    printf("\n\nDEBUG\n\n");
    printf("CONTEXT:\n");
    show_context();
    printf("\n");
    printf("FACTS\n");
    show_hash_tower(&CURR_SYSTEM.fact_set);
    printf("\n");
    printf("PROPS\n");
    show_hash_tower(&CURR_SYSTEM.prop_table);
    printf("\n");
    printf("PARAMS\n");
    show_hash_tower(&CURR_SYSTEM.used_paras);
    printf("\n");
    printf("MEMORY\n");
    show_memory_screen();
    printf("\nDEPTH: %u\n\n", CURR_SYSTEM.depth);
}

/**
 * 报错并增加错误数. 对于使用文件输入而不是 stdin 的场景, 还会添加上行号. 给定的
 * error msg 不应该包括空行.
 */
void raise_error(char *error_msg) {
    if (CURR_SYSTEM.use_file)
        printf("%s (line: %u)\n", error_msg, CURR_SYSTEM.line_cnt);
    else
        printf("%s\n", error_msg);
    CURR_SYSTEM.error_cnt++;
}

void print_wls_report() {
    printf("\nREPORT\n");
    printf("PROVE: %u/%u\n", CURR_SYSTEM.prove_achieve_cnt,
           CURR_SYSTEM.prove_try_cnt);
    printf("SUPPOSE: %u/%u\n", CURR_SYSTEM.suppose_achieve_cnt,
           CURR_SYSTEM.suppose_try_cnt);
    printf("ERROR: %u\n", CURR_SYSTEM.error_cnt);
    printf("\n");
}
