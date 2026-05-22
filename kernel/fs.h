#ifndef FS_H
#define FS_H

#define FSMAGIC 0x10203040

#define ROOTINO 1
#define HELLOINO 2

#define T_DIR 1
#define T_FILE 2

#define SBLOCK 1 //superblock 放在 block 1
#define IBLOCK 2 //inode table 放在 block 2
#define DATASTART 3 //数据区开始block

#define ROOTDIR_BLOCK 3
#define HELLO_BLOCK 4

#define NINODES 16

struct superblock {
    unsigned int magic; //文件系统魔数，用于判断是否合法
    unsigned int size; //整个 fake disk 的块数
    unsigned int nblocks; //数据块数量
    unsigned int ninodes; //inode 数量
    unsigned int inode_start; //inode table 从哪个 block 开始
    unsigned int data_start; //data blocks 从哪个 block 开始
};

struct dinode {
    int type; //inode 类型，0 表示空闲，1 表示普通文件
    unsigned int size; //文件大小，单位是字节
    unsigned int data_block; //文件内容所在的磁盘块号
};

struct dirent {
    unsigned int inum;
    char name[28];
};

void fs_init(void);
void fs_test(void);
int fs_readi(unsigned int inum, char *dst, unsigned int max);
int fs_lookup(const char *name);
int fs_readfile(const char *name, char *dst, unsigned int max);

#endif