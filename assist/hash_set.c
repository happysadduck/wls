#include "hash.h"
#include "pool.h"

#define HASH(key, bucket_cnt, out)                                             \
    do {                                                                       \
        cnt_t hash_value = 0, mask = bucket_cnt - 1;                           \
        hash_value = key * 2654435761U;                                        \
        *(out) = hash_value & mask;                                            \
    } while (0)

/**
 * @param s          set_t 指针, 位置无要求.
 * @param mem        给 set 桶数组 (moff_t 型) 提供的内存区域, 要求对4字节对齐
 * @param bucket_cnt 要分配的桶数组数量
 * @return           mem + bucket_cnt * sizeof(moff_t)
 */
char *prepare_set(set_t *s, char *mem, cnt_t bucket_cnt) {
    cnt_t i;
    s->bucket_cnt = bucket_cnt;
    s->bucket = (moff_t *)mem;
    for (i = 0; i < bucket_cnt; i++)
        s->bucket[i] = OFF(NULL);
    return mem + bucket_cnt * sizeof(moff_t);
}

/**
 * @param s   set_t 指针, 位置无要求.
 * @param mem 给新节点准备的内存区域, 从节点内存池分配
 * @param key 键
 */
void set_insert(set_t *s, set_node_t *mem, cnt_t key) {
    cnt_t idx;
    set_node_t *p;
    HASH(key, s->bucket_cnt, &idx);
    p = PTR(s->bucket[idx]);
    while (p) {
        if (p->num_key == key)
            return;
        p = PTR(p->next);
    }
    p = mem;
    p->num_key = key;
    p->next = s->bucket[idx];
    s->bucket[idx] = OFF(p);
}

int is_in_set(set_t *s, cnt_t key) {
    cnt_t idx;
    set_node_t *p;
    HASH(key, s->bucket_cnt, &idx);
    p = PTR(s->bucket[idx]);
    while (p) {
        if (p->num_key == key)
            return 1;
        p = PTR(p->next);
    }
    return 0;
}

void *set_delete(set_t *s, cnt_t key) {
    cnt_t idx;
    set_node_t *p, *prev = NULL;
    HASH(key, s->bucket_cnt, &idx);
    p = PTR(s->bucket[idx]);
    while (p) {
        if (p->num_key == key) {
            if (prev)
                prev->next = p->next;
            else
                s->bucket[idx] = p->next;
            return p;
        }
        prev = p;
        p = PTR(p->next);
    }
    return NULL;
}
