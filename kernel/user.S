    .section .text
    .globl enter_user

 /*
 * void enter_user(uint64 entry, uint64 sp)
 *
 * a0 = user entry address
 * a1 = user stack top
 */

 enter_user:
    csrw sepc, a0

    csrr t0, sstatus

    //Clear SPP so sret returns to U-mode

    li t1, -257
    and t0,t0,t1

    //set SPIE
    li t1, 32
    or t0,t0,t1

    csrw sstatus, t0

    mv sp,a1

    sret