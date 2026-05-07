#ifndef RISCV_H
#define RISCV_H

typedef unsigned long uint64;

#define SSTATUS_SPP  (1L << 8)//SSP位掩码
#define SSTATUS_SPIE (1L << 5)//SPIE位掩码

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

static inline uint64 r_sstatus(void)
{
    uint64 x;
    asm volatile("csrr %0,sstatus" : "=r"(x));
    return x;
}

static inline void w_sstatus(uint64 x)
{
    asm volatile("csrw sstatus, %0" : : "r"(0));
}

#endif