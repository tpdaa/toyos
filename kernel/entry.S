.section .text
.globl _start
.extern start

_start:
    #设置栈顶
    la sp,boot_stack_top

    #跳转到c语言入口start()
    call start

1:
    #如果 start()返回，就停在这里
    j 1b

    .section .bss
    .align 12
boot_stack:
    .space 4096
boot_stack_top: