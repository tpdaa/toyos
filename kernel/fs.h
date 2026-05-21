#ifndef FS_H
#define FS_H

#define FSMAGIC 0x10203040

#define ROOTINO 1
#define T_FILE 1

#define SBLOCK 1
#define IBLOCK 2
#define DATASTART 3

#define NINODES 16

struct superblock {
    unsigned int magic;
    unsigned int size;
    unsigned int nblocks;
    unsigned int ninodes;
    unsigned int inode_start;
    unsigned int data_start;
};

struct dinode {
    int type;
    unsigned int size;
    unsigned int data_block;
};

void fs_init(void);
void fs_test(void);

#endif