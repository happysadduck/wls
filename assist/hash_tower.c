#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "pool.h"
#include "token.h"

#define HASH(str, bucket_cnt, out)                                             \
    do {                                                                       \
        char *p = (char *)str;                                                 \
        int hash_value = 0, mask = bucket_cnt - 1;                             \
        while (*p) {                                                           \
            hash_value = hash_value * 31 + *p++;                               \
        }                                                                      \
        *out = hash_value & mask;                                              \
    } while (0)

char *prepare_layer_manager(layer_manager_t *m, char *mem_set,
                            cnt_t layer_cnt) {
    m->curr_layer = NULL;
    m->layer_max = 0;
    return prepare_set(&m->using_lvs_set, mem_set, layer_cnt);
}

/**
 * @param t          hash_tower_t 指针, 位置无要求.
 * @param mem        给 tower 桶数组 (moff_t 型) 提供的内存区域, 要求对4字节对齐
 * @param bucket_cnt 要分配的桶数组数量
 * @param m          对应控制器, 位置无要求
 * @return           mem + bucket_cnt * sizeof(moff_t)
 */
char *prepare_tower(hash_tower_t *t, char *mem, cnt_t bucket_cnt,
                    layer_manager_t *m) {
    cnt_t i;
    t->bucket = (moff_t *)mem;
    t->bucket_cnt = bucket_cnt;
    t->manager = m;
    for (i = 0; i < bucket_cnt; i++)
        t->bucket[i] = OFF(NULL);
    return mem + bucket_cnt * sizeof(moff_t);
}

int tower_insert(hash_tower_t *t, tower_node_t *mem, char *key,
                 void *value_data) {
    cnt_t idx;
    tower_node_t *p;
    HASH(key, t->bucket_cnt, &idx);
    p = PTR(t->bucket[idx]);
    while (p) {
        char *pkey = PTR(p->key);
        if (!is_in_set(&t->manager->using_lvs_set, p->layer_create)) {
            p->key = OFF(key);
            p->data = OFF(value_data);
            p->layer_create = t->manager->curr_layer->lv;
            return 1;
        }
        if (pkey && strcmp(key, pkey) == 0)
            return 0;
        p = PTR(p->next);
    }
    p = mem;
    p->key = OFF(key);
    p->data = OFF(value_data);
    p->layer_create = t->manager->curr_layer->lv;
    p->next = t->bucket[idx];
    t->bucket[idx] = OFF(p);
    return 1;
}

void *tower_find(hash_tower_t *t, const char *key) {
    cnt_t idx;
    tower_node_t *p;
    HASH(key, t->bucket_cnt, &idx);
    p = PTR(t->bucket[idx]);
    while (p) {
        char *pkey = PTR(p->key);
        if (is_in_set(&t->manager->using_lvs_set, p->layer_create) && pkey &&
            strcmp(key, pkey) == 0)
            return PTR(p->data);
        p = PTR(p->next);
    }
    return NULL;
}

int is_in_tower(hash_tower_t *t, const char *key) {
    cnt_t idx;
    tower_node_t *p;
    HASH(key, t->bucket_cnt, &idx);
    p = PTR(t->bucket[idx]);
    while (p) {
        char *pkey = PTR(p->key);
        if (is_in_set(&t->manager->using_lvs_set, p->layer_create) && pkey &&
            strcmp(key, pkey) == 0)
            return 1;
        p = PTR(p->next);
    }
    return 0;
}

void *tower_delete(hash_tower_t *t, const char *key) {
    cnt_t idx;
    tower_node_t *p, *prev = NULL;
    HASH(key, t->bucket_cnt, &idx);
    p = PTR(t->bucket[idx]);
    while (p) {
        char *pkey = PTR(p->key);
        if (is_in_set(&t->manager->using_lvs_set, p->layer_create) && pkey &&
            strcmp(key, pkey) == 0) {
            if (prev)
                prev->next = p->next;
            else
                t->bucket[idx] = p->next;
            return p;
        }
        prev = p;
        p = PTR(p->next);
    }
    return NULL;
}

/**
 * @param m                层控制器
 * @param mem_layer        给 layer_t 准备的内存区域, 需要 sizeof(layer_t)
 * 的空间, 并且要求4字节对齐
 * @param mem_set_node set 所需的节点内存, 从 pool_for_nodes 里面分配.
 */
void tower_add_layer(layer_manager_t *m, char *mem_layer,
                     set_node_t *mem_set_node) {
    layer_t *new_layer = (layer_t *)mem_layer;
    new_layer->lv = ++(m->layer_max);
    new_layer->prev = OFF(m->curr_layer);
    m->curr_layer = new_layer;
    set_insert(&m->using_lvs_set, mem_set_node, new_layer->lv);
}

void *tower_cut_layer(layer_manager_t *m, set_node_t **hanged_set_node) {
    layer_t *old_layer = m->curr_layer;
    m->curr_layer = PTR(old_layer->prev);
    *hanged_set_node = set_delete(&m->using_lvs_set, old_layer->lv);
    return old_layer;
}
