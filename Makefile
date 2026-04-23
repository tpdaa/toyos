CROSS = riscv64-linux-gnu-
CC = $(CROSS)gcc

CFLAGS = -Wall -Werror -O0 -g -nostdlib -nostartfiles -ffreestanding -fno-pie -no-pie -mcmodel=medany
LDFLAGS = -T kernel/linker.ld -static -nostdlib -nostartfiles -no-pie -Wl,--build-id=none

KERNEL_OBJS = \
	kernel/entry.o \
	kernel/start.o \
	kernel/sbi.o \
	kernel/printf.o
	
all: kernel.elf
kernel.elf: $(KERNEL_OBJS) kernel/linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

kernel/entry.o: kernel/entry.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/start.o: kernel/start.c kernel/sbi.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/sbi.o: kernel/sbi.c kernel/sbi.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/printf.o:kernel/printf.c kernel/printf.h  kernel/sbi.h
	$(CC) $(CFLAGS) -c -o $@ $<

run : kernel.elf
	qemu-system-riscv64 \
	-machine virt \
	-nographic \
	-bios default \
	-kernel kernel.elf 

clean:
	rm -f kernel/*.o kernel.elf
