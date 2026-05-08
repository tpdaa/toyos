#ifndef MEMLATOUT_H
#define MEMLAYOUT_H

#define PGSIZE 4096UL

#define KERNBASE 0x80200000UL
#define PHYSTOP  0x88000000UL

#define PGROUNDUP(sz)   (((sz)+PGSIZE-1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a)  ((a) & ~(PGSIZE - 1))
 
#endif