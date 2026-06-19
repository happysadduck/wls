#ifndef PTR_H
#define PTR_H

#include <stddef.h>

typedef unsigned int moff_t;
typedef unsigned int cnt_t;

extern char *MEM_BASE;
extern char *TOP;

#define PTR(off) (void *)((off) == 0 ? NULL : MEM_BASE + (off - 1))
#define OFF(ptr) ((ptr) == NULL ? 0 : (char *)(ptr) - MEM_BASE + 1)
#define ALI(ptr, align)                                                        \
    ((ptr) = (void *)((((size_t)(ptr) + (align) - 1) & ~((size_t)(align) - 1))))

#endif
