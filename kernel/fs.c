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

static unsigned int strlen_fs(const char *s)
{
    unsigned int n = 0;

    while (s[n] != '\0') 
    {
        n++;
    }

    return n;
}

static int streq_fs(const char *a, const char *b)
{
    unsigned int i = 0;

    while (a[i] != '\0' && b[i] != '\0') 
    {
        if (a[i] != b[i]) 
        {
            return 0;
        }
        i++;
    }

    return a[i] == '\0' && b[i] == '\0';
}

void fs_init(void)
{
    unsigned char buf[BSIZE];
    struct superblock sb;
    struct dinode *dip;
    struct dirent *de;
    const char *hello_msg = "hello from toyfs\n";
    const char *readme_msg = "this is a tiny file system\n";
    const char *hello_name = "hello.txt";
    const char *readme_name = "readme.txt";

    unsigned int hello_len = strlen_fs(hello_msg);
    unsigned int readme_len = strlen_fs(readme_msg);
    unsigned int hello_namelen = strlen_fs(hello_name);
    unsigned int readme_namelen = strlen_fs(readme_name);

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
    dip[ROOTINO].type = T_DIR;
    dip[ROOTINO].size = 2 * sizeof(struct dirent);
    dip[ROOTINO].data_block = ROOTDIR_BLOCK;

    dip[HELLOINO].type = T_FILE;
    dip[HELLOINO].size = hello_len;
    dip[HELLOINO].data_block = HELLO_BLOCK;

    dip[READMEINO].type = T_FILE;
    dip[READMEINO].size = readme_len;
    dip[READMEINO].data_block = README_BLOCK;

    if (block_write(IBLOCK, buf) < 0) 
    {
        printf("fs init failed: write inode table failed\n");
        return;
    }

    /*
     * root directory block.
     */
    memzero_fs(buf, BSIZE);
    de = (struct dirent *)buf;

    /*
    * directory entry 0: hello.txt
    */
    de[0].inum = HELLOINO;

    if (hello_namelen >= sizeof(de[0].name)) {
        hello_namelen = sizeof(de[0].name) - 1;
    }

    memcopy_fs(de[0].name, hello_name, hello_namelen);
    de[0].name[hello_namelen] = '\0';

    /*
    * directory entry 1: readme.txt
    */
    de[1].inum = READMEINO;

    if (readme_namelen >= sizeof(de[1].name)) 
    {
        readme_namelen = sizeof(de[1].name) - 1;
    }

    memcopy_fs(de[1].name, readme_name, readme_namelen);
    de[1].name[readme_namelen] = '\0';

    if (block_write(ROOTDIR_BLOCK, buf) < 0) 
    {
        printf("fs init failed: write root directory failed\n");
        return;
    }

    /*
    * hello.txt data block.
    */
    memzero_fs(buf, BSIZE);
    memcopy_fs(buf, hello_msg, hello_len);

    if (block_write(HELLO_BLOCK, buf) < 0) 
    {
        printf("fs init failed: write hello data failed\n");
        return;
    }

    /*
    * readme.txt data block.
    */
    memzero_fs(buf, BSIZE);
    memcopy_fs(buf, readme_msg, readme_len);

    if (block_write(README_BLOCK, buf) < 0) 
    {
        printf("fs init failed: write readme data failed\n");
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

int fs_lookup(const char *name)
{
    unsigned char buf[BSIZE];
    struct dinode *dip;
    struct dinode rootino;
    struct dirent *de;
    unsigned int nentry;

    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_lookup: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;
    rootino = dip[ROOTINO];

    if (rootino.type != T_DIR) 
    {
        printf("fs_lookup: root is not directory\n");
        return -1;
    }

    if (rootino.size > BSIZE) 
    {
        printf("fs_lookup: root directory too large\n");
        return -1;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(rootino.data_block, buf) < 0) 
    {
        printf("fs_lookup: read root dir failed\n");
        return -1;
    }

    de = (struct dirent *)buf;
    nentry = rootino.size / sizeof(struct dirent);

    for (unsigned int i = 0; i < nentry; i++) 
    {
        if (de[i].inum != 0 && streq_fs(de[i].name, name)) 
        {
            return de[i].inum;
        }
    }

    return -1;
}

int fs_readfile(const char *name, char *dst, unsigned int max)
{
    int inum = fs_lookup(name);

    if (inum < 0) 
    {
        printf("fs_readfile: file not found: %s\n", name);
        return -1;
    }

    return fs_readi((unsigned int)inum, dst, max);
}

void fs_test(void)
{
    unsigned char buf[BSIZE];
    unsigned char filebuf[BSIZE];
    unsigned char readmebuf[BSIZE];
    struct superblock sb;
    struct dinode *dip;
    struct dinode rootino;

    memzero_fs(buf, BSIZE);
    memzero_fs(filebuf, BSIZE);
    memzero_fs(readmebuf, BSIZE);
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

    if (rootino.type != T_DIR)
    {
        printf("fs test failed: bad root inode type=%d\n", rootino.type);
        return;
    }

    if (rootino.data_block != ROOTDIR_BLOCK) 
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
    n = fs_readfile("hello.txt", (char *)filebuf, BSIZE - 1);
    if (n < 0) 
    {
        printf("fs test failed: fs_readi failed\n");
        return;
    }
    filebuf[n] = '\0';

    int n2;
    n2 = fs_readfile("readme.txt", (char *)readmebuf, BSIZE - 1);
    if (n2 < 0) 
    {
        printf("fs test failed: read readme.txt failed\n");
        return;
    }
    readmebuf[n2] = '\0';

    printf("fs test passed. size=%d nblocks=%d ninodes=%d root_data=%d read_n=%d\n",
       sb.size, sb.nblocks, sb.ninodes, rootino.data_block, n);

    printf("fs file content: %s", filebuf);
    printf("fs readme content: %s", readmebuf);

}