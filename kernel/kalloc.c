#include "kalloc.h"
#include "memlayout.h"
#include "riscv.h"
#include "printf.h"

extern char end[];

struct run {
    struct run *next;
};

static struct {
    struct run *freelist;
} kmem;

static void memset_page(void *pa,int value)
{
    unsigned char *p = (unsigned char *)pa;

    for(int i = 0; i< PGSIZE; i++){
        p[i] = value;
    }
}

static void freerange(void *pa_start,void *pa_end)
{
    char *p = (char *)PGROUNDUP((uint64)pa_start);

    for(;p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    {
        kfree(p);
    }
}

void kinit(void)
{
    freerange(end,(void *)PHYSTOP);

    printf("[MM] kalloc init free=%p-%p\n",
            (void *)PGROUNDUP((uint64)end),(void *)PHYSTOP);
}

void kfree(void *pa)
{
    struct run *r;

    if(((uint64)pa % PGSIZE) != 0)
    {
        printf("kfree: address not page aligned: %p\n", pa);
        return;
    }

    if((uint64)pa < PGROUNDUP((uint64)end) || (uint64)pa >= PHYSTOP)
    {
        printf("kfree: invalid address: %p\n", pa);
        return;
    }

    memset_page(pa, 1);

    r = (struct run *)pa;
    r->next = kmem.freelist;
    kmem.freelist = r;
}

void *kalloc(void)
{
    struct run *r;
    r = kmem.freelist;
    if(r)
    {
        kmem.freelist = r->next;
        memset_page((void *)r,5);
    }

    return (void *)r;
}
