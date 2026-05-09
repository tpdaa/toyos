#ifndef TRAPFRAME_H
#define TRAPFRAME_H

#include "riscv.h"

struct trapframe
{
    uint64 ra;      //0
    uint64 gp;      // 8
    uint64 tp;      // 16
    uint64 t0;      // 24
    uint64 t1;      // 32
    uint64 t2;      // 40
    uint64 s0;      // 48
    uint64 s1;      // 56
    uint64 a0;      // 64
    uint64 a1;      // 72
    uint64 a2;      // 80
    uint64 a3;      // 88
    uint64 a4;      // 96
    uint64 a5;      // 104
    uint64 a6;      // 112
    uint64 a7;      // 120
    uint64 s2;      // 128
    uint64 s3;      // 136
    uint64 s4;      // 144
    uint64 s5;      // 152
    uint64 s6;      // 160
    uint64 s7;      // 168
    uint64 s8;      // 176
    uint64 s9;      // 184
    uint64 s10;     // 192
    uint64 s11;     // 200
    uint64 t3;      // 208
    uint64 t4;      // 216
    uint64 t5;      // 224
    uint64 t6;      // 232

    uint64 sp; //240, user sp saved from sscratch
    uint64 reserved1; //248
};

#if defined(__cplusplus)
static_assert(sizeof(struct trapframe) == 256,
              "trapframe size must match trap.S stack frame");
#else
_Static_assert(sizeof(struct trapframe) == 256,
               "trapframe size must match trap.S stack frame");
#endif
#endif