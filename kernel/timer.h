#ifndef TIMER_H
#define TIMER_H

#include "riscv.h"

void timer_init(void);
void timer_tick(void);
uint64 timer_ticks(void);

#endif