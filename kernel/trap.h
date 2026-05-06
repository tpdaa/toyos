#ifndef TRAP_H
#define TRAP_H

struct trapframe;

void trap_init(void);
void kernel_trap(struct trapframe *tf);

#endif