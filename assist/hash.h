#ifndef HASH_H
#define HASH_H

#include "ptr.h"

typedef struct {
    cnt_t num_key;
    moff_t next;
} set_node_t;
typedef struct {
    moff_t *bucket;
    cnt_t bucket_cnt;
} set_t;

typedef struct {
    moff_t key;
    moff_t data;
    cnt_t layer_create;
    moff_t next;
} tower_node_t;

typedef struct {
    cnt_t lv;
    moff_t prev;
} layer_t;

typedef struct {
    cnt_t layer_max;
    layer_t *curr_layer;
    set_t using_lvs_set;
} layer_manager_t;

typedef struct {
    cnt_t bucket_cnt;
    moff_t *bucket;
    layer_manager_t *manager;
} hash_tower_t;

char *prepare_set(set_t *set, char *mem, cnt_t bucket_cnt);

void set_insert(set_t *s, set_node_t *mem, cnt_t key);
int is_in_set(set_t *s, cnt_t key);
void *set_delete(set_t *s, cnt_t key);

char *prepare_layer_manager(layer_manager_t *m, char *mem_set, cnt_t layer_cnt);

char *prepare_tower(hash_tower_t *tower, char *mem, cnt_t bucket_cnt,
                    layer_manager_t *manager);

int tower_insert(hash_tower_t *tower, tower_node_t *mem, char *key,
                 void *value_data);
void *tower_find(hash_tower_t *tower, const char *key);
int is_in_tower(hash_tower_t *tower, const char *key);
void *tower_delete(hash_tower_t *tower, const char *key);

void tower_add_layer(layer_manager_t *manager, char *mem_layer,
                     set_node_t *mem_set_node);
void *tower_cut_layer(layer_manager_t *manager, set_node_t **hanged_set_node);

#endif
