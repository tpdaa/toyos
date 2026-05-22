#include "fs.h"
#include "block.h"
#include "printf.h"

static void memzero_fs(void *dst, unsigned int n)
{
    unsigned char *p = (unsigned char *)dst;

    for (unsigned int i = 0; i < n; i++) 
    {
        p[i] = 0;
    }
}

static void memcopy_fs(void *dst, const void *src, unsigned int n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    for (unsigned int i = 0; i < n; i++) 
    {
        d[i] = s[i];
    }
}

void fs_init(void)
{
    unsigned char buf[BSIZE];
    struct superblock sb;
    struct dinode *dip;
    const char *msg = "hello from toyfs\n";
    unsigned int len = 0;

    while (msg[len] != '\0') 
    {
        len++;
    }

    memzero_fs(buf, BSIZE);
    memzero_fs(&sb, sizeof(sb));

    sb.magic = FSMAGIC;
    sb.size = NBLOCKS;
    sb.nblocks = NBLOCKS - DATASTART;
    sb.ninodes = NINODES;
    sb.inode_start = IBLOCK;
    sb.data_start = DATASTART;

    memcopy_fs(buf, &sb, sizeof(sb));

    if (block_write(SBLOCK, buf) < 0) 
    {
        printf("fs init failed: write superblock failed\n");
        return;
    }

    memzero_fs(buf, BSIZE);

    dip = (struct dinode *)buf;
    dip[ROOTINO].type = T_FILE;
    dip[ROOTINO].size = len;
    dip[ROOTINO].data_block = DATASTART;

    if (block_write(IBLOCK, buf) < 0) 
    {
        printf("fs init failed: write inode table failed\n");
        return;
    }

    memzero_fs(buf, BSIZE);
    memcopy_fs(buf, msg, len);

    if (block_write(DATASTART, buf) < 0) 
    {
        printf("fs init failed: write data block failed\n");
        return;
    }

    printf("fs init done. magic=0x%x\n", sb.magic);
}

int fs_readi(unsigned int inum, char *dst, unsigned int max)
{
    unsigned char buf[BSIZE];
    struct dinode *dip;
    struct dinode ino;
    unsigned int n;

    if (inum >= NINODES) 
    {
        printf("fs_readi: bad inum %d\n", inum);
        return -1;
    }

    if (max == 0) 
    {
        return 0;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_readi: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;
    ino = dip[inum];

    if (ino.type != T_FILE) 
    {
        printf("fs_readi: inode %d is not file\n", inum);
        return -1;
    }

    if (ino.size == 0) 
    {
        return 0;
    }

    if (ino.size > BSIZE) 
    {
        printf("fs_readi: file too large size=%d\n", ino.size);
        return -1;
    }

    n = ino.size;

    if (n > max) 
    {
        n = max;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(ino.data_block, buf) < 0) 
    {
        printf("fs_readi: read data block failed\n");
        return -1;
    }

    memcopy_fs(dst, buf, n);

    return n;
}

void fs_test(void)
{
    unsigned char buf[BSIZE];
    unsigned char filebuf[BSIZE];
    struct superblock sb;
    struct dinode *dip;
    struct dinode rootino;

    memzero_fs(buf, BSIZE);
    memzero_fs(filebuf, BSIZE);
    memzero_fs(&sb, sizeof(sb));
    memzero_fs(&rootino, sizeof(rootino));

    if (block_read(SBLOCK, buf) < 0) 
    {
        printf("fs test failed: read superblock failed\n");
        return;
    }

    memcopy_fs(&sb, buf, sizeof(sb));

    if (sb.magic != FSMAGIC) 
    {
        printf("fs test failed: bad magic=0x%x\n", sb.magic);
        return;
    }

    if (sb.size != NBLOCKS) 
    {
        printf("fs test failed: bad size=%d\n", sb.size);
        return;
    }

    if (sb.inode_start != IBLOCK) 
    {
        printf("fs test failed: bad inode_start=%d\n", sb.inode_start);
        return;
    }

    if (sb.data_start != DATASTART) 
    {
        printf("fs test failed: bad data_start=%d\n", sb.data_start);
        return;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs test failed: read inode table failed\n");
        return;
    }

    dip = (struct dinode *)buf;
    rootino = dip[ROOTINO];

    if (rootino.type != T_FILE) 
    {
        printf("fs test failed: bad root inode type=%d\n", rootino.type);
        return;
    }

    if (rootino.data_block != DATASTART) 
    {
        printf("fs test failed: bad root data_block=%d\n", rootino.data_block);
        return;
    }

    if (rootino.size >= BSIZE) 
    {
        printf("fs test failed: bad root size=%d\n", rootino.size);
        return;
    }

    int n;
    n = fs_readi(ROOTINO, (char *)filebuf, BSIZE - 1);
    if (n < 0) 
    {
        printf("fs test failed: fs_readi failed\n");
        return;
    }
    filebuf[n] = '\0';

    printf("fs test passed. size=%d nblocks=%d ninodes=%d root_data=%d root_size=%d read_n=%d\n",
           sb.size, sb.nblocks, sb.ninodes, rootino.data_block, rootino.size, n);

    printf("fs file content: %s", filebuf);

}