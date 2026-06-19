#include "pool.h"
#include <stddef.h>

char *prepare_pool(pool_t *pool, char *mem, cnt_t size, cnt_t cnt) {
    cnt_t block_sz;
    char *curr;
    char *next;
    cnt_t i;

    block_sz = (size >= sizeof(void *)) ? size : (cnt_t)sizeof(void *);
    pool->free_list = (void *)mem;
    pool->block_size = block_sz;
    pool->alloc_cnt = 0;
    pool->mem_start = mem;

    curr = mem;
    for (i = 0; i < cnt - 1; i++) {
        next = curr + block_sz;
        *(void **)curr = (void *)next;
        curr = next;
    }
    *(void **)curr = NULL;

    return mem + cnt * block_sz;
}

void *pool_alloc(pool_t *pool) {
    void *block;
    pool->alloc_cnt++;
    if (pool->free_list == NULL) {
        return NULL;
    }

    block = pool->free_list;
    pool->free_list = *(void **)block;
    return block;
}

void pool_return(pool_t *pool, void *ptr) {
    pool->alloc_cnt--;
    *(void **)ptr = pool->free_list;
    pool->free_list = ptr;
}
