OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
    . = 0x80200000;

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    }

    . = ALIGN(4096);
    PROVIDE(user_start = .);

    .user : {
        *(.user.text .user.text.*)
        *(.user.rodata .user.rodata.*)
        *(.user.data .user.data.*)
        *(.user.bss .user.bss.*)
    }

    . = ALIGN(4096);
    PROVIDE(user_end = .);

    . = ALIGN(4096);
    PROVIDE(end = .);
}