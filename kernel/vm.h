#ifndef VM_H
#define VM_H

#include "riscv.h"

typedef uint64 pte_t;
typedef uint64 *pagetable_t;

//RISC-V Sv39 PTE flags

#define PTE_V (1L << 0)
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4)

#define PTE_FLAGS(pte) ((pte) & 0x3FF)

#define PA2PTE(pa) ((((uint64)(pa)) >>12) << 10)
#define PTE2PA(pte) (((pte) >>10) <<12)

#define PXSHIFT(level) (12 + (9 * (level)))
#define PX(level, va) ((((uint64)(va)) >> PXSHIFT(level)) & 0x1FF)


#define MAXVA (1L << (9+9+9+12-1))

#define SATP_SV39 (8L <<60)
#define MAKE_SATP(pagetable) (SATP_SV39 | (((uint64)(pagetable)) >>12))

extern pagetable_t kernel_pagetable;

pte_t *walk(pagetable_t pagetable, uint64 va, int alloc);
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm);

void kvminit(void);
void kvminithart(void);

void vm_switch(pagetable_t pagetable);

struct proc;
void uvminit(struct proc *p);
void uvminithart(pagetable_t pagetable);

uint64 walkaddr(pagetable_t pagetable, uint64 va);

int copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len);
int copyout(pagetable_t pagetable, uint64 dstva, const char *src, uint64 len);
int copystr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max);

void vmprint_pte(uint64 va);

pagetable_t uvmcreate(void);
int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz);

#endif