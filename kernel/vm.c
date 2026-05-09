#include "vm.h"
#include "memlayout.h"
#include "kalloc.h"
#include "printf.h"

pagetable_t kernel_pagetable;

extern char user_start[];
extern char user_end[];

static void memset_bytes(void *dst, int value, uint64 n)
{
    unsigned char *p = (unsigned char *)dst;

    for (uint64 i = 0; i < n; i++)
    {
        p[i] = value;
    }
}

pte_t *walk(pagetable_t pagetable, uint64 va, int alloc)
{
    if(va>=MAXVA)
    {
        printf("walk: va too large: %p\n",(void *)va);
        return 0;
    }

    for(int level =2; level >0; level--)
    {
       pte_t *pte = &pagetable[PX(level, va)];

       if(*pte & PTE_V)
       {
            pagetable = (pagetable_t)PTE2PA(*pte);
       }
       else
       {
            if(!alloc)
            {
                return 0;
            }

            pagetable_t new_pagetable = (pagetable_t)kalloc();

            if(new_pagetable == 0)
            {
                return 0;
            }

            memset_bytes(new_pagetable, 0, PGSIZE);

            *pte = PA2PTE(new_pagetable) | PTE_V;

            pagetable = new_pagetable;
       }
    }
    return &pagetable[PX(0,va)];
}

int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
    uint64 a;
    uint64 last;

    if(size == 0)
    {
        return -1;
    }

    a = PGROUNDDOWN(va);
    last = PGROUNDDOWN(va + size -1);

    for(;;)
    {
        pte_t *pte = walk(pagetable, a ,1);
        if(pte ==0)
        {
            return -1;
        }

        if(*pte & PTE_V)
        {
            printf("mappages: remap va=%p\n",(void *)a);
            return -1;
        }

        *pte = PA2PTE(pa) | perm | PTE_V;

        if(a == last)
        {
            break;
        }

        a += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

static void kvmmap(uint64 va, uint64 pa, uint64 size, int perm)
{
    if(mappages(kernel_pagetable, va, size, pa, perm) != 0)
    {
        printf("kvmmap: failed va=%p pa=%p size=%lx\n",(void *)va,(void *)pa,size);

        for(;;)
        {asm volatile("wfi");}
    }
}

void kvminit(void)
{
    kernel_pagetable = (pagetable_t)kalloc();

    if(kernel_pagetable == 0)
    {
        printf("kvminit: kalloc failed\n");

        for(;;)
        {asm volatile("wfi");}
    }

    memset_bytes(kernel_pagetable,0, PGSIZE);

    uint64 us = PGROUNDDOWN((uint64)user_start);
    uint64 ue = PGROUNDUP((uint64)user_end);

    //Map kernel memory before user section
    if(us > KERNBASE)
    {
        kvmmap(KERNBASE, KERNBASE, us-KERNBASE, PTE_R|PTE_W|PTE_X);
    }

    kvmmap(us, us, ue - us, PTE_R | PTE_W | PTE_X | PTE_U);

    //Map the remaining RAM after user section
    if(ue <PHYSTOP)
    {
        kvmmap(ue, ue, PHYSTOP - ue, PTE_R | PTE_W | PTE_X);
    }

    printf("kvminit done. kernel_pagetable=%p user=%p-%p\n",
           (void *)kernel_pagetable, (void *)us, (void *)ue);
}

void kvminithart(void)
{
    uint64 satp = MAKE_SATP(kernel_pagetable);

    w_satp(satp);
    sfence_vma();

    w_sstatus(r_sstatus() | SSTATUS_SUM);

    printf("paging enabled. satp=%lx\n",satp);
}
