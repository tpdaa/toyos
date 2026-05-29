.section .text
.globl _start
.extern start

//OpenSBI 跳入 ToyOS 后执行的第一条内核代码
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