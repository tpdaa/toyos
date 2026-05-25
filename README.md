# ToyOS

ToyOS 是一个基于 **RISC-V64** 的教学型操作系统内核，运行在 **QEMU virt** 机器上，通过 **OpenSBI** 启动进入 S-mode。.​:codex-file-citation[codex-file-citation]{line_range_start=1 line_range_end=5 path=README.md git_url="https://github.com/tpdaa/toyos/blob/main/README.md#L1-L5"}​

项目目标是从最小启动链路出发，逐步实现操作系统核心机制：  
- 启动与异常处理（trap）  
- 系统调用与用户态切换  
- 物理内存分配与 Sv39 页表  
- 进程与调度  
- 简单块设备与文件系统  
- 文件相关系统调用与基础“进程文件描述符表”​:codex-file-citation[codex-file-citation]{line_range_start=3 line_range_end=7 path=README.md git_url="https://github.com/tpdaa/toyos/blob/main/README.md#L3-L7"}​​:codex-file-citation[codex-file-citation]{line_range_start=5 line_range_end=23 path=kernel/syscall.h git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.h#L5-L23"}​

---

## 一、项目结构（核心模块）

- `kernel/entry.S`：内核入口（早期启动）。  
- `kernel/start.c`：内核主初始化流程。  
- `kernel/trap.S + kernel/trap.c`：trap 现场保存恢复、trap 分发与返回。  
- `kernel/syscall.c`：syscall 分发与具体系统调用实现。  
- `kernel/kalloc.c`：物理页分配器。  
- `kernel/vm.c`：Sv39 页表、映射、用户内存访问与地址空间释放。  
- `kernel/proc.c`：进程管理、调度、fork/wait/exit/exec。  
- `kernel/block.c`：内存模拟块设备。  
- `kernel/fs.c`：最小文件系统（inode + dirent + bitmap）。​:codex-file-citation[codex-file-citation]{line_range_start=1 line_range_end=10 path=kernel/start.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/start.c#L1-L10"}​​:codex-file-citation[codex-file-citation]{line_range_start=1 line_range_end=8 path=kernel/trap.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/trap.c#L1-L8"}​​:codex-file-citation[codex-file-citation]{line_range_start=1 line_range_end=5 path=kernel/syscall.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.c#L1-L5"}​​:codex-file-citation[codex-file-citation]{line_range_start=1 line_range_end=7 path=kernel/vm.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/vm.c#L1-L7"}​​:codex-file-citation[codex-file-citation]{line_range_start=1 line_range_end=4 path=kernel/proc.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/proc.c#L1-L4"}​​:codex-file-citation[codex-file-citation]{line_range_start=1 line_range_end=4 path=kernel/block.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/block.c#L1-L4"}​​:codex-file-citation[codex-file-citation]{line_range_start=1 line_range_end=4 path=kernel/fs.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/fs.c#L1-L4"}​

---

## 二、启动与初始化流程

内核从 `start()` 开始，按顺序完成：

1. `trap_init()`：设置 `stvec` 等 trap 基础设施。  
2. `kinit()`：初始化物理页分配器。  
3. `kvminit()`：建立内核页表。  
4. `procinit()`：初始化进程表。  
5. `block_init()/block_test()`：初始化并测试块设备。  
6. `fs_init()/fs_test()`：初始化并测试文件系统。  
7. `userinit()`：创建首个用户进程。  
8. `timer_init()`：开启时钟。  
9. `scheduler()`：进入调度循环。​:codex-file-citation[codex-file-citation]{line_range_start=12 line_range_end=45 path=kernel/start.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/start.c#L12-L45"}​

---

## 三、Trap 机制（用户态/内核态统一入口）

### 1) trap 入口与栈切换
`trap_entry` 使用 `sscratch` 判定 trap 来源：
- `sscratch != 0`：来自用户态，先切到内核栈；
- `sscratch == 0`：来自内核态，不切栈。  
这样避免“用户态 trap 仍在用户栈上执行内核处理”的风险。.​:codex-file-citation[codex-file-citation]{line_range_start=10 line_range_end=18 path=kernel/trap.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/trap.c#L10-L18"}​

### 2) trap 处理逻辑
`kernel_trap()` 当前处理：
- **时钟中断**：调用 `timer_tick()`，必要时触发 `yield()` 抢占调度。  
- **U-mode ecall (`scause=8`)**：进入 `syscall()` 分发。  
- 其他异常：打印信息并停机等待。​:codex-file-citation[codex-file-citation]{line_range_start=30 line_range_end=76 path=kernel/trap.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/trap.c#L30-L76"}​​:codex-file-citation[codex-file-citation]{line_range_start=78 line_range_end=127 path=kernel/trap.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/trap.c#L78-L127"}​

---

## 四、系统调用子系统

### 1) 调用约定
- `a7`：syscall 编号  
- `a0~a2`：参数  
- `a0`：返回值  
由 `syscall(struct trapframe *tf)` 统一分发。.​:codex-file-citation[codex-file-citation]{line_range_start=6 line_range_end=23 path=kernel/syscall.h git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.h#L6-L23"}​​:codex-file-citation[codex-file-citation]{line_range_start=473 line_range_end=560 path=kernel/syscall.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.c#L473-L560"}​

### 2) 当前已实现 syscall

#### 进程相关
- `SYS_exit`
- `SYS_yield`
- `SYS_getpid`
- `SYS_fork`
- `SYS_wait`
- `SYS_exec`
- `SYS_getprogid`​:codex-file-citation[codex-file-citation]{line_range_start=6 line_range_end=14 path=kernel/syscall.h git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.h#L6-L14"}​​:codex-file-citation[codex-file-citation]{line_range_start=482 line_range_end=522 path=kernel/syscall.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.c#L482-L522"}​

#### 文件名/内容级接口
- `SYS_readfile`
- `SYS_listfiles`
- `SYS_createfile`
- `SYS_statfile`
- `SYS_unlinkfile`
- `SYS_writefile`
- `SYS_appendfile`​:codex-file-citation[codex-file-citation]{line_range_start=14 line_range_end=23 path=kernel/syscall.h git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.h#L14-L23"}​​:codex-file-citation[codex-file-citation]{line_range_start=42 line_range_end=286 path=kernel/syscall.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.c#L42-L286"}​

#### 文件描述符级接口
- `SYS_openfile`
- `SYS_readfd`
- `SYS_writefd`
- `SYS_closefd`​:codex-file-citation[codex-file-citation]{line_range_start=19 line_range_end=23 path=kernel/syscall.h git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.h#L19-L23"}​​:codex-file-citation[codex-file-citation]{line_range_start=288 line_range_end=470 path=kernel/syscall.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.c#L288-L470"}​

---

## 五、进程与调度

### 1) 进程模型
内核维护固定大小进程表（`NPROC`），进程状态包括：
`UNUSED/USED/SLEEPING/RUNNABLE/RUNNING/ZOMBIE`。  
进程含内核栈、trapframe、context、父子关系、文件表等。.​:codex-file-citation[codex-file-citation]{line_range_start=8 line_range_end=63 path=kernel/proc.h git_url="https://github.com/tpdaa/toyos/blob/main/kernel/proc.h#L8-L63"}​

### 2) 调度模型
`scheduler()` 扫描 `RUNNABLE` 进程，切换到该进程页表，设置 `sscratch` 为该进程内核栈顶，然后 `swtch` 切到进程上下文。首次运行通过 `forkret()` 进入用户态返回路径。.​:codex-file-citation[codex-file-citation]{line_range_start=312 line_range_end=369 path=kernel/proc.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/proc.c#L312-L369"}​

### 3) `fork/wait/exit`
- `fork`：复制页表与 trapframe，子进程 `a0=0`，父进程返回子 pid。  
- `wait`：父进程回收 `ZOMBIE` 子进程并释放其地址空间。  
- `exit`：子进程转 `ZOMBIE`，唤醒父进程。  
- 支持 `reparent`（孤儿交给 `initproc`）。​:codex-file-citation[codex-file-citation]{line_range_start=394 line_range_end=566 path=kernel/proc.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/proc.c#L394-L566"}​

### 4) fork 失败路径资源回收（已增强）
你新增了 `freeproc_partial()`，在 `allocproc/uvmcreate/uvmcopy` 等失败路径统一释放半成品子进程资源（含 `uvmfree`），避免内存泄漏。.​:codex-file-citation[codex-file-citation]{line_range_start=132 line_range_end=172 path=kernel/proc.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/proc.c#L132-L172"}​

---

## 六、内存管理与 Sv39

### 1) 物理页分配器
- 4KB 页粒度  
- freelist 管理空闲页  
- `kinit()/kalloc()/kfree()`​:codex-file-citation[codex-file-citation]{line_range_start=7 line_range_end=67 path=kernel/kalloc.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/kalloc.c#L7-L67"}​

### 2) 页表与映射
- 支持 `walk/mappages/walkaddr`  
- 建立内核页表并写 `satp` 开分页  
- 使用 `sfence.vma` 刷新 TLB​:codex-file-citation[codex-file-citation]{line_range_start=24 line_range_end=140 path=kernel/vm.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/vm.c#L24-L140"}​

### 3) 用户地址空间
- `uvmcreate()` 创建用户页表（包含内核高地址映射，不带 `PTE_U`）  
- `uvminit()` 把 `.user` 镜像拷贝到低地址用户空间  
- 维护 `entry/stack_top/user_pc/sz`​:codex-file-citation[codex-file-citation]{line_range_start=147 line_range_end=278 path=kernel/vm.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/vm.c#L147-L278"}​

### 4) 用户内存安全访问
- `copyin/copyout/copystr`  
- 通过页表检查权限，复制期间临时设置 `SSTATUS_SUM`，结束后恢复。.​:codex-file-citation[codex-file-citation]{line_range_start=364 line_range_end=487 path=kernel/vm.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/vm.c#L364-L487"}​

---

## 七、块设备与文件系统

### 1) 块设备层
`block.c` 以内存数组模拟磁盘（`fake_disk[NBLOCKS][BSIZE]`），提供 `block_read/write`。并有 `block_test()` 做读写一致性检查。.​:codex-file-citation[codex-file-citation]{line_range_start=4 line_range_end=80 path=kernel/block.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/block.c#L4-L80"}​

### 2) 文件系统层
`fs_init()` 会初始化：
- superblock  
- inode bitmap / data bitmap  
- inode 表（根目录 + 初始文件）  
- 根目录项和文件数据块  

初始示例文件包含 `hello.txt` 与 `readme.txt`。.​:codex-file-citation[codex-file-citation]{line_range_start=94 line_range_end=258 path=kernel/fs.c git_url="https://github.com/tpdaa/toyos/blob/main/kernel/fs.c#L94-L258"}​

---

## 八、用户程序与 program id

通过 `SYS_exec` + `program_id` 机制加载不同用户程序，当前定义了 shell/hello/count/cat/ls/create/stat/unlink/fdtest/write/append/fdwrite 等编号。.​:codex-file-citation[codex-file-citation]{line_range_start=25 line_range_end=36 path=kernel/syscall.h git_url="https://github.com/tpdaa/toyos/blob/main/kernel/syscall.h#L25-L36"}​

---

## 九、编译与运行

```bash
make
make run