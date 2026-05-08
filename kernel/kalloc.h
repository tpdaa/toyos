#ifndef KALLOC_H
#define KALLOC_H

void kinit(void);
void *kalloc(void);
void kfree(void *pa);

#endif