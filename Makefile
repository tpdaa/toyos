CROSS = riscv64-linux-gnu-
CC = $(CROSS)gcc

CFLAGS = -std=gnu11 -Wall -Werror -O0 -g -nostdlib -nostartfiles -ffreestanding -fno-pie -no-pie -mcmodel=medany
LDFLAGS = -T kernel/linker.ld -static -nostdlib -nostartfiles -no-pie -Wl,--build-id=none

KERNEL_OBJS = \
	kernel/entry.o \
	kernel/start.o \
	kernel/sbi.o \
	kernel/printf.o \
	kernel/trap.o \
	kernel/trap_entry.o \
	kernel/syscall.o \
	kernel/user.o \
	kernel/user_entry.o \
	kernel/kalloc.o \
	kernel/vm.o
	
all: kernel.elf
kernel.elf: $(KERNEL_OBJS) kernel/linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

kernel/entry.o: kernel/entry.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/start.o: kernel/start.c kernel/printf.h kernel/trap.h kernel/kalloc.h kernel/riscv.h kernel/user.h kernel/vm.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/sbi.o: kernel/sbi.c kernel/sbi.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/printf.o: kernel/printf.c kernel/printf.h  kernel/sbi.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/trap.o: kernel/trap.c kernel/trap.h kernel/trapframe.h kernel/riscv.h kernel/printf.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/trap_entry.o: kernel/trap.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/syscall.o: kernel/syscall.c kernel/syscall.h kernel/trapframe.h kernel/printf.h kernel/vm.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/user.o: kernel/user.c kernel/user.h kernel/syscall.h 
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/user_entry.o: kernel/user.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/kalloc.o: kernel/kalloc.c kernel/kalloc.h kernel/memlayout.h kernel/riscv.h kernel/printf.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/vm.o: kernel/vm.c kernel/vm.h kernel/memlayout.h kernel/kalloc.h kernel/printf.h kernel/riscv.h
	$(CC) $(CFLAGS) -c -o $@ $<

run : kernel.elf
	qemu-system-riscv64 \
	-machine virt \
	-display none \
	-serial stdio \
	-monitor none \
	-bios default \
	-kernel kernel.elf 

clean:
	rm -f kernel/*.o kernel.elf
