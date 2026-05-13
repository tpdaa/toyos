#ifndef RISCV_H
#define RISCV_H

typedef unsigned long uint64;

#define SSTATUS_SPP  (1L << 8)//SSP位掩码
#define SSTATUS_SPIE (1L << 5)//SPIE位掩码
#define SSTATUS_SUM  (1L << 18)
#define SSTATUS_SIE (1L << 1)

#define SIE_STIE (1L << 5)

#define SCAUSE_INTERRUPT (1ULL << 63)
#define SCAUSE_TIMER     5

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
    asm volatile("csrr %0, sstatus" : "=r"(x));
    return x;
}

static inline void w_sstatus(uint64 x)
{
    asm volatile("csrw sstatus, %0" : : "r"(x));
}

static inline uint64 r_sscratch(void)
{
    uint64 x;
    asm volatile("csrr %0, sscratch" : "=r"(x));
    return x;
}

static inline void w_sscratch(uint64 x)
{
    asm volatile("csrw sscratch, %0" : : "r"(x));
}

static inline void w_satp(uint64 x)
{
    asm volatile("csrw satp, %0" : : "r"(x));
}

static inline void sfence_vma(void)
{
    asm volatile("sfence.vma");
}

static inline void set_sstatus(uint64 x)
{
    asm volatile("csrs sstatus, %0" : : "r"(x));
}

static inline void clear_sstatus(uint64 x)
{
    asm volatile("csrc sstatus, %0" : : "r"(x));
}

static inline uint64
r_sie(void)
{
    uint64 x;
    asm volatile("csrr %0, sie" : "=r" (x));
    return x;
}

static inline void
w_sie(uint64 x)
{
    asm volatile("csrw sie, %0" : : "r" (x));
}

#endif