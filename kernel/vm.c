#include "vm.h"
#include "memlayout.h"
#include "kalloc.h"
#include "printf.h"
#include "user.h"
#include "proc.h"

pagetable_t kernel_pagetable;

extern char user_start[];
extern char user_end[];

//用于清零页表页
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

static void map_or_panic(pagetable_t pagetable,
                         uint64 va,
                         uint64 pa,
                         uint64 size,
                         int perm)
{
    if (mappages(pagetable, va, size, pa, perm) != 0) {
        printf("map_or_panic: failed va=%p pa=%p size=%lx\n",
               (void *)va, (void *)pa, size);

        for (;;) {
            asm volatile("wfi");
        }
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

    map_or_panic(kernel_pagetable, KERNBASE, KERNBASE, PHYSTOP - KERNBASE,
                         PTE_R|PTE_W|PTE_X);

    printf("[VM] init kernel_pagetable=%p\n",
           (void *)kernel_pagetable);
}

void vm_switch(pagetable_t pagetable)
{
    uint64 satp = MAKE_SATP(pagetable);

    w_satp(satp);
    sfence_vma();
}

void kvminithart(void)
{
    vm_switch(kernel_pagetable);
    printf("paging enabled. satp=%lx\n", MAKE_SATP(kernel_pagetable));
}

static void copy_bytes(char *dst, const char *src, uint64 n)
{
    for (uint64 i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

pagetable_t uvmcreate(void)
{
    pagetable_t pagetable = (pagetable_t)kalloc();

    if (pagetable == 0)
    {
        return 0;
    }

    memset_bytes(pagetable, 0, PGSIZE);

    /*
     * 用户页表也映射内核高地址，但不带 PTE_U。
     * 这样 trap 进入内核后，S-mode 仍能执行内核代码。
     */
    if (mappages(pagetable,
                 KERNBASE,
                 PHYSTOP - KERNBASE,
                 KERNBASE,
                 PTE_R | PTE_W | PTE_X) != 0)
    {
        return 0;
    }

    return pagetable;
}

int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
    for (uint64 off = 0; off < sz; off += PGSIZE)
    {
        uint64 va = USERBASE + off;

        pte_t *pte = walk(old, va, 0);

        if (pte == 0)
        {
            printf("uvmcopy: pte not found va=%p\n", (void *)va);
            return -1;
        }

        if ((*pte & PTE_V) == 0)
        {
            printf("uvmcopy: pte not valid va=%p\n", (void *)va);
            return -1;
        }

        if ((*pte & PTE_U) == 0)
        {
            /*
             * 这里只复制用户页。
             */
            continue;
        }

        uint64 pa = PTE2PA(*pte);
        int flags = PTE_FLAGS(*pte);

        char *mem = (char *)kalloc();

        if (mem == 0)
        {
            printf("uvmcopy: kalloc failed\n");
            return -1;
        }

        copy_bytes(mem, (const char *)pa, PGSIZE);

        if (mappages(new,
                     va,
                     PGSIZE,
                     (uint64)mem,
                     flags & ~PTE_V) != 0)
        {
            printf("uvmcopy: mappages failed va=%p\n", (void *)va);
            return -1;
        }
    }

    return 0;
}

void uvminit(struct proc *p)
{
    uint64 image_start = (uint64)user_start;
    uint64 image_end = (uint64)user_end;
    uint64 image_size = image_end - image_start;
    uint64 image_pages = PGROUNDUP(image_size);

    p->pagetable = uvmcreate();

    if (p->pagetable == 0)
    {
        printf("uvminit: uvmcreate failed\n");
        for (;;)
        {
            asm volatile("wfi");
        }
    }

    /*
     * Copy user image from kernel .user section to newly allocated pages,
     * then map those pages at low user virtual addresses.
     */
    for (uint64 off = 0; off < image_pages; off += PGSIZE) {
        char *mem = (char *)kalloc();

        if (mem == 0) {
            printf("uvminit: kalloc user page failed\n");
            for (;;) {
                asm volatile("wfi");
            }
        }

        memset_bytes(mem, 0, PGSIZE);

        uint64 n = PGSIZE;

        if (off + n > image_size) {
            n = image_size - off;
        }

        copy_bytes(mem, (const char *)(image_start + off), n);

        map_or_panic(p->pagetable,
                     USERBASE + off,
                     (uint64)mem,
                     PGSIZE,
                     PTE_R | PTE_W | PTE_X | PTE_U);
    }

    p->entry = USERBASE + ((uint64)user_main - image_start);
    p->stack_top = USERBASE + ((uint64)user_stack - image_start) + USER_STACK_SIZE;


    p->user_pc = p->entry;
    p->trapframe.sp = p->stack_top;
    p->sz = image_pages;

    printf("uvminit done. user_pagetable=%p entry=%p stack_top=%p image_size=%lx\n",
           (void *)p->pagetable,
           (void *)p->entry,
           (void *)p->stack_top,
           image_size);
}

void uvminithart(pagetable_t pagetable)
{
    vm_switch(pagetable);

    // printf("switched to user_pagetable. satp=%lx\n",
    //        MAKE_SATP(pagetable));
}

static uint64 walkaddr_perm(pagetable_t pagetable, uint64 va, int perm)
{
    pte_t *pte;

    if(va >= MAXVA)
    {
        return 0;
    }

    pte = walk(pagetable, va, 0);

    if(pte ==0)
    {
        return 0;
    }

    if ((*pte & PTE_V) == 0) {
        return 0;
    }

    if ((*pte & PTE_U) == 0) {
        return 0;
    }

    if ((*pte & perm) != perm) {
        return 0;
    }

    return PTE2PA(*pte);
}

uint64 walkaddr(pagetable_t pagetable, uint64 va)
{
    return walkaddr_perm(pagetable, va ,PTE_R);
}

static void memmove_bytes(char *dst, const char *src, uint64 n)
{
    for (uint64 i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}



int copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
    uint64 n;
    uint64 va0;
    uint64 pa0;
    uint64 old_sstatus;

    old_sstatus = r_sstatus();
    set_sstatus(SSTATUS_SUM);

    while (len > 0) {
        va0 = PGROUNDDOWN(srcva);
        pa0 = walkaddr_perm(pagetable, va0, PTE_R);

        if (pa0 == 0) {
            w_sstatus(old_sstatus);
            return -1;
        }

        n = PGSIZE - (srcva - va0);

        if (n > len) {
            n = len;
        }

        memmove_bytes(dst, (const char *)(pa0 + (srcva - va0)), n);

        len -= n;
        dst += n;
        srcva = va0 + PGSIZE;
    }

    w_sstatus(old_sstatus);
    return 0;
}

int copyout(pagetable_t pagetable, uint64 dstva, const char *src, uint64 len)
{
    uint64 n;
    uint64 va0;
    uint64 pa0;
    uint64 old_sstatus;

    old_sstatus = r_sstatus();
    set_sstatus(SSTATUS_SUM);

    while (len > 0) 
    {
        va0 = PGROUNDDOWN(dstva);
        pa0 = walkaddr_perm(pagetable, va0, PTE_W);

        if (pa0 == 0) 
        {
            w_sstatus(old_sstatus);
            return -1;
        }

        n = PGSIZE - (dstva - va0);

        if (n > len) 
        {
            n = len;
        }

        memmove_bytes((char *)(pa0 + (dstva - va0)), src, n);

        len -= n;
        src += n;
        dstva = va0 + PGSIZE;
    }

    w_sstatus(old_sstatus);
    return 0;
}

int copystr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max)
{
    uint64 va0;
    uint64 pa0;
    uint64 n;
    uint64 old_sstatus;
    char *p;

    if (max == 0)     
    {
        return -1;
    }

    old_sstatus = r_sstatus();
    set_sstatus(SSTATUS_SUM);

    while(max > 0)
    {
        va0 = PGROUNDDOWN(srcva);
        pa0 = walkaddr_perm(pagetable, va0, PTE_R);

        if (pa0 == 0) 
        {
            w_sstatus(old_sstatus);
            return -1;
        }

        n = PGSIZE - (srcva - va0);

        if (n > max) 
        {
            n = max;
        }

        p = (char *)(pa0 + (srcva - va0));

        while (n > 0) 
        {
            *dst = *p;

            if (*dst == '\0') 
            {
                w_sstatus(old_sstatus);
                return 0;
            }

            dst++;
            p++;
            n--;
            max--;
        }

        srcva = va0 + PGSIZE;
    }

    w_sstatus(old_sstatus);
    return -1;
}

void vmprint_pte(uint64 va)
{
    pte_t *pte = walk(kernel_pagetable, va, 0);

    if (pte == 0) {
        printf("vmprint_pte: va=%p pte=0\n", (void *)va);
        return;
    }

    printf("vmprint_pte: va=%p pte=%lx pa=%p flags=%lx\n",
           (void *)va,
           *pte,
           (void *)PTE2PA(*pte),
           PTE_FLAGS(*pte));
}

void uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
{
    if ((va % PGSIZE) != 0)
    {
        printf("uvmunmap: va not aligned %p\n", (void *)va);
        return;
    }

    for (uint64 i = 0; i < npages; i++)
    {
        uint64 a = va + i * PGSIZE;
        pte_t *pte = walk(pagetable, a, 0);

        if (pte == 0)
        {
            continue;
        }

        if ((*pte & PTE_V) == 0)
        {
            continue;
        }

        /*
         * 这里只应该释放用户页。
         * 内核映射不能 kfree。
         */
        if ((*pte & PTE_U) == 0)
        {
            continue;
        }

        if (do_free)
        {
            uint64 pa = PTE2PA(*pte);
            kfree((void *)pa);
        }

        *pte = 0;
    }
}

static void freewalk(pagetable_t pagetable)
{
    for (int i = 0; i < 512; i++)
    {
        pte_t pte = pagetable[i];

        if ((pte & PTE_V) == 0)
        {
            continue;
        }

        /*
         * 如果这个 PTE 指向下一级页表，而不是叶子页，
         * 就递归释放下一级页表。
         */
        if ((pte & (PTE_R | PTE_W | PTE_X)) == 0)
        {
            uint64 child = PTE2PA(pte);
            freewalk((pagetable_t)child);
            pagetable[i] = 0;
        }
        else
        {
            /*
             * 叶子映射直接清掉。
             *
             * 用户页的物理内存已经由 uvmunmap() 释放；
             * 内核映射不能释放物理页，只能清映射。
             */
            pagetable[i] = 0;
        }
    }

    kfree((void *)pagetable);
}

void uvmfree(pagetable_t pagetable, uint64 sz)
{
    if (pagetable == 0)
    {
        return;
    }

    if (sz > 0)
    {
        uint64 npages = PGROUNDUP(sz) / PGSIZE;
        uvmunmap(pagetable, USERBASE, npages, 1);
    }

    freewalk(pagetable);
}
