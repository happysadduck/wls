#ifndef POOL_H
#define POOL_H

#include "ptr.h"

typedef struct {
    void *free_list;
    cnt_t block_size;
    cnt_t alloc_cnt;
    char *mem_start;
} pool_t;

char *prepare_pool(pool_t *pool, char *mem, cnt_t size, cnt_t cnt);
void *pool_alloc(pool_t *pool);
void pool_return(pool_t *pool, void *ptr);

#endif
