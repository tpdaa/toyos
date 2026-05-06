#ifndef RISCV_H
#define RISCV_H

typedef unsigned long uint64;

static inline uint64 r_scause(void)
{
    uint64 x;
    asm volatile("csrr %0,scause" : "=r"(x));
    return x;
}

static inline uint64 r_sepc(void)
{
     uint64 x;
    asm volatile("csrr %0, sepc" : "=r"(x));
    return x;
} 

static inline void w_sepc(uint64 x)
{
    asm volatile("csrw sepc, %0" : : "r"(x));
}

static inline uint64 r_stval(void)
{
    uint64 x;
    asm volatile("csrr %0, stval" : "=r"(x));
    return x;
}

static inline void w_stvec(uint64 x)
{
    asm volatile("csrw stvec, %0" : : "r"(x));
}


#endif