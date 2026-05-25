#ifndef FS_H
#define FS_H

#define FSMAGIC 0x10203040

#define ROOTINO 1
#define HELLOINO 2
#define READMEINO 3

#define T_DIR 1
#define T_FILE 2

#define SBLOCK 1 //superblock 放在 block 1
#define IBITMAP_BLOCK 2
#define DBITMAP_BLOCK 3
#define IBLOCK 4 //inode table 放在 block 4
#define DATASTART 5 //数据区开始block

#define ROOTDIR_BLOCK 5 // 根目录内容所在 block
#define HELLO_BLOCK 6  // hello.txt 文件内容所在 block
#define README_BLOCK 7 // readme.txt 文件内容所在 block

#define NINODES 16

struct superblock {
    unsigned int magic; //文件系统魔数，用于判断是否合法
    unsigned int size; //整个 fake disk 的块数
    unsigned int nblocks; //数据块数量
    unsigned int ninodes; //inode 数量
    unsigned int inode_bitmap_start;
    unsigned int data_bitmap_start;
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

struct filestat {
    unsigned int inum;
    unsigned int type;
    unsigned int size;
    unsigned int data_block;
};

void fs_init(void);
void fs_test(void);
int fs_readi(unsigned int inum, char *dst, unsigned int max);
int fs_lookup(const char *name);
int fs_readfile(const char *name, char *dst, unsigned int max);
int fs_stat(const char *name, struct filestat *st);
int fs_list(char *dst, unsigned int max);
int fs_create(const char *name, const char *content);
int fs_unlink(const char *name);
int fs_writefile(const char *name, const char *content);
int fs_open(const char *name);
int fs_readi_at(unsigned int inum, char *dst, unsigned int max, unsigned int off);
int fs_appendfile(const char *name, const char *content);

#endif