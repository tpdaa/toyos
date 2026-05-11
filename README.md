# ToyOS

ToyOS 是一个基于 RISC-V64 的教学型操作系统内核项目，目前运行在 QEMU virt 机器上，通过 OpenSBI 启动进入 S-mode。

本项目目标是从最小内核启动开始，逐步实现 trap、系统调用、物理内存管理、页表、用户态程序、进程与文件系统等操作系统核心机制。

---

## 当前已实现功能

### 1. 内核启动

- 使用 `entry.S` 作为内核入口。
- 设置启动栈后进入 `start()`。
- 通过 linker script 将内核加载到 `0x80200000`。
- 能够在 QEMU + OpenSBI 环境下正常启动。

### 2. SBI 输出与内核 printf

- 实现了 `sbi_call()`。
- 实现了 `sbi_putchar()`。
- 实现了简化版内核 `printf()`。
- 支持基本格式化输出，例如：
  - `%c`
  - `%s`
  - `%d`
  - `%x`
  - `%p`
  - `%ld`

### 3. Trap 机制

- 设置 `stvec` 指向 `trap_entry`。
- 能够处理基本异常。
- 实现了 `trap.S` 保存和恢复寄存器现场。
- 实现了 `struct trapframe`。
- 支持从 trap 返回继续执行。
- 使用 `sscratch` 区分用户态 trap 和内核态 trap。
- 用户态 trap 进入内核后会切换到内核 trap 栈，避免继续使用用户栈。

### 4. 最小系统调用框架

- 实现了 syscall 分发器。
- 使用 RISC-V 常见约定：
  - `a7` 保存 syscall number
  - `a0-a2` 保存参数
  - `a0` 保存返回值
- 当前支持：
  - `SYS_puts`
  - `SYS_exit`

### 5. 用户态执行

- 能够通过 `sret` 从 S-mode 进入 U-mode。
- 实现了 `enter_user(entry, sp)`。
- 用户态通过 `ecall` 进入内核。
- 内核能够识别 `scause = 8`，即 U-mode ecall。

### 6. 物理页分配器

- 实现了物理页分配器：
  - `kinit()`
  - `kalloc()`
  - `kfree()`
- 使用 4KB 页作为基本分配单位。
- 使用空闲链表管理物理页。
- 空闲页本身作为链表节点保存 `next` 指针。
- 支持基本的分配、释放和复用测试。

### 7. Sv39 页表与分页

- 定义了 Sv39 页表相关结构：
  - `pte_t`
  - `pagetable_t`
  - `PTE_V`
  - `PTE_R`
  - `PTE_W`
  - `PTE_X`
  - `PTE_U`
- 实现了：
  - `walk()`
  - `mappages()`
  - `walkaddr()`
- 实现了 `kernel_pagetable`。
- 能够写入 `satp` 开启分页。
- 使用 `sfence.vma` 刷新地址转换缓存。

### 8. 用户内存安全访问

- 实现了：
  - `copyin()`
  - `copyout()`
  - `copyinstr()`
- `sys_puts` 不再直接使用用户指针。
- 用户字符串会先通过 `copyinstr()` 复制到内核缓冲区，再由内核打印。
- 临时使用 `SSTATUS_SUM` 允许 S-mode 在复制过程中访问用户页，并在复制结束后恢复状态。

### 9. 低地址用户空间

- `.user` 段现在作为用户程序镜像保存在内核中。
- 启动时通过 `uvminit()` 将用户镜像复制到新分配的物理页。
- 将用户程序映射到低地址用户虚拟空间：
  - `USERBASE = 0x1000`
- 实现了独立的 `user_pagetable` 雏形。
- 用户程序现在从低地址执行，例如：
  - `entry = 0x1040`
  - syscall 时 `sepc = 0x1026`
- 这说明用户程序已经不再直接运行在内核高地址 `.user` 段中。

---

## 当前运行结果示例

```text
Toyos kernel start.
trap init done, stvec=0x80200758 kstack=0x80205000
kalloc init done. free memory: 0x8020b000 - 0x88000000
kalloc test: p1=0x87fff000 p2=0x87ffe000
kalloc test: after kfree(p1), p3=0x87fff000
kvminit done. kernel_pagetable=0x87fff000
uvminit done. user_pagetable=0x87fbe000 entry=0x1040 stack_top=0x50a0 image_size=5000
switched to user_pagetable. satp=8000000000087fbe
enter user mode...
trap happened: scause=8 sepc=0x1026 stval=0
syscall: num=1
Hello from U-mode via syscall!
trap happened: scause=8 sepc=0x1026 stval=0
syscall: num=2
user exit, code=0

