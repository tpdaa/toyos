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

    .section .text
    .globl usertrapret

/*
 * void usertrapret(struct trapframe *tf, uint64 user_pc, uint64 kstack_top);
 *
 * a0 = tf
 * a1 = user_pc
 * a2 = kernel stack top
 *
 * 从指定 trapframe 恢复用户态寄存器，然后 sret 回 U-mode。
 */
usertrapret:
    mv t0, a0

    /*
     * 设置返回用户态后的 PC。
     */
    csrw sepc, a1

    /*
     * 清 SPP，使 sret 返回 U-mode。
     * 置 SPIE，使回到 U-mode 后允许中断。
     */
    csrr t1, sstatus

    li t2, -257
    and t1, t1, t2

    li t2, 32
    or t1, t1, t2

    csrw sstatus, t1

    /*
     * 回到用户态后，sscratch 必须保存当前进程的 kernel stack top。
     * 下次用户态 trap 时，trap_entry 会用它切换到内核栈。
     */
    csrw sscratch, a2

    /*
     * 从 trapframe 恢复用户寄存器。
     * 注意：t0 当前作为 trapframe 基地址，最后再恢复原来的 t0。
     */
    ld ra, 0(t0)
    ld gp, 8(t0)
    ld tp, 16(t0)

    ld t1, 32(t0)
    ld t2, 40(t0)

    ld s0, 48(t0)
    ld s1, 56(t0)

    ld a0, 64(t0)
    ld a1, 72(t0)
    ld a2, 80(t0)
    ld a3, 88(t0)
    ld a4, 96(t0)
    ld a5, 104(t0)
    ld a6, 112(t0)
    ld a7, 120(t0)

    ld s2, 128(t0)
    ld s3, 136(t0)
    ld s4, 144(t0)
    ld s5, 152(t0)
    ld s6, 160(t0)
    ld s7, 168(t0)
    ld s8, 176(t0)
    ld s9, 184(t0)
    ld s10, 192(t0)
    ld s11, 200(t0)

    ld t3, 208(t0)
    ld t4, 216(t0)
    ld t5, 224(t0)
    ld t6, 232(t0)

    /*
     * 恢复用户 sp。
     */
    ld sp, 240(t0)

    /*
     * 最后恢复用户原来的 t0。
     */
    ld t0, 24(t0)

    sret