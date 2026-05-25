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

static unsigned int appendstr_fs(char *dst, unsigned int off, unsigned int max, const char *s)
{
    unsigned int i = 0;

    while (s[i] != '\0') 
    {
        if (off + 1 >= max) 
        {
            break;
        }

        dst[off] = s[i];
        off++;
        i++;
    }

    return off;
}

static void copy_name_to_dirent(char *dst, const char *name)
{
    unsigned int i = 0;

    while (name[i] != '\0' && i + 1 < 28) 
    {
        dst[i] = name[i];
        i++;
    }

    dst[i] = '\0';
}

static void bitmap_set(unsigned char *bitmap, unsigned int bit)
{
    bitmap[bit / 8] = bitmap[bit / 8] | (1 << (bit % 8));
}

static int bitmap_get(unsigned char *bitmap, unsigned int bit)
{
    return (bitmap[bit / 8] >> (bit % 8)) & 1;
}

static void bitmap_clear(unsigned char *bitmap, unsigned int bit)
{
    bitmap[bit / 8] = bitmap[bit / 8] & ~(1 << (bit % 8));
}

static int bitmap_find_free(unsigned char *bitmap, unsigned int nbits)
{
    for (unsigned int i = 1; i < nbits; i++) 
    {
        if (!bitmap_get(bitmap, i)) 
        {
            return (int)i;
        }
    }

    return -1;
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
    sb.inode_bitmap_start = IBITMAP_BLOCK;
    sb.data_bitmap_start = DBITMAP_BLOCK;
    sb.inode_start = IBLOCK;
    sb.data_start = DATASTART;

    memcopy_fs(buf, &sb, sizeof(sb));

    if (block_write(SBLOCK, buf) < 0) 
    {
        printf("fs init failed: write superblock failed\n");
        return;
    }

    /*
    * inode bitmap.
    * inode 1: root directory
    * inode 2: hello.txt
    * inode 3: readme.txt
    */
    memzero_fs(buf, BSIZE);

    bitmap_set(buf, ROOTINO);
    bitmap_set(buf, HELLOINO);
    bitmap_set(buf, READMEINO);

    if (block_write(IBITMAP_BLOCK, buf) < 0) 
    {
        printf("fs init failed: write inode bitmap failed\n");
        return;
    }

    /*
    * data bitmap.
    * data block 5: root directory
    * data block 6: hello.txt
    * data block 7: readme.txt
    */
    memzero_fs(buf, BSIZE);

    bitmap_set(buf, ROOTDIR_BLOCK - DATASTART);
    bitmap_set(buf, HELLO_BLOCK - DATASTART);
    bitmap_set(buf, README_BLOCK - DATASTART);

    if (block_write(DBITMAP_BLOCK, buf) < 0) 
    {
        printf("fs init failed: write data bitmap failed\n");
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

    if (fs_create("note.txt", "created by fs_create\n") < 0)
    {
        printf("fs init warning: create note.txt failed\n");
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

int
fs_stat(const char *name, struct filestat *st)
{
    unsigned char buf[BSIZE];
    struct dinode *dip;
    int inum;

    inum = fs_lookup(name);
    if (inum < 0) 
    {
        printf("fs_stat: file not found: %s\n", name);
        return -1;
    }

    if ((unsigned int)inum >= NINODES) 
    {
        printf("fs_stat: bad inum=%d\n", inum);
        return -1;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_stat: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;

    st->inum = (unsigned int)inum;
    st->type = (unsigned int)dip[inum].type;
    st->size = dip[inum].size;
    st->data_block = dip[inum].data_block;

    return 0;
}

int fs_list(char *dst, unsigned int max)
{
    unsigned char buf[BSIZE];
    struct dinode *dip;
    struct dinode rootino;
    struct dirent *de;
    unsigned int nentry;
    unsigned int off = 0;

    if (max == 0) 
    {
        return 0;
    }

    memzero_fs(dst, max);
    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_list: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;
    rootino = dip[ROOTINO];

    if (rootino.type != T_DIR) 
    {
        printf("fs_list: root is not directory\n");
        return -1;
    }

    if (rootino.size > BSIZE) 
    {
        printf("fs_list: root directory too large\n");
        return -1;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(rootino.data_block, buf) < 0) 
    {
        printf("fs_list: read root dir failed\n");
        return -1;
    }

    de = (struct dirent *)buf;
    nentry = rootino.size / sizeof(struct dirent);

    for (unsigned int i = 0; i < nentry; i++) 
    {
        if (de[i].inum == 0) 
        {
            continue;
        }

        off = appendstr_fs(dst, off, max, de[i].name);
        off = appendstr_fs(dst, off, max, "\n");
    }

    if (off < max) 
    {
        dst[off] = '\0';
    } 
    else 
    {
        dst[max - 1] = '\0';
    }

    return off;
}

int fs_create(const char *name, const char *content)
{
    unsigned char buf[BSIZE];
    unsigned char ibitmap[BSIZE];
    unsigned char dbitmap[BSIZE];
    struct dinode *dip;
    struct dinode rootino;
    struct dirent *de;
    int free_inum;
    int free_data_index;
    unsigned int free_block;
    unsigned int content_len;
    unsigned int nentry;

    /*
     * 如果文件已经存在，就不重复创建。
     */
    if (fs_lookup(name) >= 0) 
    {
        printf("fs_create: file already exists: %s\n", name);
        return -1;
    }

    content_len = strlen_fs(content);
    if (content_len > BSIZE) 
    {
        printf("fs_create: content too large size=%d\n", content_len);
        return -1;
    }

    /*
     * 读取 inode bitmap，找空闲 inode。
     */
    memzero_fs(ibitmap, BSIZE);

    if (block_read(IBITMAP_BLOCK, ibitmap) < 0) 
    {
        printf("fs_create: read inode bitmap failed\n");
        return -1;
    }

    free_inum = bitmap_find_free(ibitmap, NINODES);
    if (free_inum < 0) 
    {
        printf("fs_create: no free inode\n");
        return -1;
    }

    /*
     * 读取 data bitmap，找空闲 data block。
     */
    memzero_fs(dbitmap, BSIZE);

    if (block_read(DBITMAP_BLOCK, dbitmap) < 0) 
    {
        printf("fs_create: read data bitmap failed\n");
        return -1;
    }

    free_data_index = bitmap_find_free(dbitmap, NBLOCKS - DATASTART);
    if (free_data_index < 0) 
    {
        printf("fs_create: no free data block\n");
        return -1;
    }

    free_block = DATASTART + (unsigned int)free_data_index;

    /*
     * 读取 inode table，写入新 inode。
     */
    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_create: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;

    dip[free_inum].type = T_FILE;
    dip[free_inum].size = content_len;
    dip[free_inum].data_block = free_block;

    rootino = dip[ROOTINO];

    if (rootino.type != T_DIR) 
    {
        printf("fs_create: root is not directory\n");
        return -1;
    }

    if (rootino.size + sizeof(struct dirent) > BSIZE) 
    {
        printf("fs_create: root directory full\n");
        return -1;
    }

    if (block_write(IBLOCK, buf) < 0) 
    {
        printf("fs_create: write inode table failed\n");
        return -1;
    }

    /*
     * 写入文件内容。
     */
    memzero_fs(buf, BSIZE);
    memcopy_fs(buf, content, content_len);

    if (block_write(free_block, buf) < 0) 
    {
        printf("fs_create: write file data failed\n");
        return -1;
    }

    /*
     * 更新 root directory，追加一个 dirent。
     */
    memzero_fs(buf, BSIZE);

    if (block_read(ROOTDIR_BLOCK, buf) < 0) 
    {
        printf("fs_create: read root directory failed\n");
        return -1;
    }

    de = (struct dirent *)buf;
    nentry = rootino.size / sizeof(struct dirent);

    de[nentry].inum = (unsigned int)free_inum;
    copy_name_to_dirent(de[nentry].name, name);

    if (block_write(ROOTDIR_BLOCK, buf) < 0) 
    {
        printf("fs_create: write root directory failed\n");
        return -1;
    }

    /*
     * 更新 root inode 的目录大小。
     */
    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_create: reread inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;
    dip[ROOTINO].size = rootino.size + sizeof(struct dirent);

    if (block_write(IBLOCK, buf) < 0) 
    {
        printf("fs_create: update root inode failed\n");
        return -1;
    }

    /*
     * 更新 inode bitmap 和 data bitmap。
     */
    bitmap_set(ibitmap, (unsigned int)free_inum);

    if (block_write(IBITMAP_BLOCK, ibitmap) < 0) 
    {
        printf("fs_create: write inode bitmap failed\n");
        return -1;
    }

    bitmap_set(dbitmap, (unsigned int)free_data_index);

    if (block_write(DBITMAP_BLOCK, dbitmap) < 0) 
    {
        printf("fs_create: write data bitmap failed\n");
        return -1;
    }

    printf("fs_create: created %s inum=%d block=%d size=%d\n",
           name, free_inum, free_block, content_len);

    return 0;
}

int fs_writefile(const char *name, const char *content)
{
    unsigned char buf[BSIZE];
    struct dinode *dip;
    int inum;
    unsigned int content_len;
    unsigned int data_block;

    inum = fs_lookup(name);
    if (inum < 0) 
    {
        printf("fs_writefile: file not found: %s\n", name);
        return -1;
    }

    if ((unsigned int)inum >= NINODES) 
    {
        printf("fs_writefile: bad inum=%d\n", inum);
        return -1;
    }

    content_len = strlen_fs(content);
    if (content_len > BSIZE) 
    {
        printf("fs_writefile: content too large size=%d\n", content_len);
        return -1;
    }

    /*
     * 读取 inode table，找到目标文件 inode。
     */
    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_writefile: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;

    if (dip[inum].type != T_FILE) 
    {
        printf("fs_writefile: not a file: %s\n", name);
        return -1;
    }

    data_block = dip[inum].data_block;

    if (data_block < DATASTART || data_block >= NBLOCKS) 
    {
        printf("fs_writefile: bad data block=%d\n", data_block);
        return -1;
    }

    /*
     * 更新 inode.size。
     * data_block 不变，因为这是覆盖写，不重新分配块。
     */
    dip[inum].size = content_len;

    if (block_write(IBLOCK, buf) < 0) 
    {
        printf("fs_writefile: write inode table failed\n");
        return -1;
    }

    /*
     * 覆盖写文件内容。
     */
    memzero_fs(buf, BSIZE);
    memcopy_fs(buf, content, content_len);

    if (block_write(data_block, buf) < 0) 
    {
        printf("fs_writefile: write data block failed\n");
        return -1;
    }

    printf("fs_writefile: wrote %s inum=%d block=%d size=%d\n",
           name, inum, data_block, content_len);

    return 0;
}

int fs_appendfile(const char *name, const char *content)
{
    unsigned char buf[BSIZE];
    unsigned char data[BSIZE];
    struct dinode *dip;
    int inum;
    unsigned int old_size;
    unsigned int append_len;
    unsigned int new_size;
    unsigned int data_block;

    inum = fs_lookup(name);
    if (inum < 0) 
    {
        printf("fs_appendfile: file not found: %s\n", name);
        return -1;
    }

    if ((unsigned int)inum >= NINODES) 
    {
        printf("fs_appendfile: bad inum=%d\n", inum);
        return -1;
    }

    append_len = strlen_fs(content);

    /*
     * 读取 inode table。
     */
    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_appendfile: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;

    if (dip[inum].type != T_FILE) 
    {
        printf("fs_appendfile: not a file: %s\n", name);
        return -1;
    }

    old_size = dip[inum].size;
    data_block = dip[inum].data_block;

    if (old_size > BSIZE) 
    {
        printf("fs_appendfile: bad old size=%d\n", old_size);
        return -1;
    }

    if (append_len > BSIZE || old_size + append_len > BSIZE) 
    {
        printf("fs_appendfile: file too large old=%d append=%d\n",
               old_size, append_len);
        return -1;
    }

    if (data_block < DATASTART || data_block >= NBLOCKS) 
    {
        printf("fs_appendfile: bad data block=%d\n", data_block);
        return -1;
    }

    /*
     * 读取原文件内容所在 data block。
     */
    memzero_fs(data, BSIZE);

    if (block_read(data_block, data) < 0) 
    {
        printf("fs_appendfile: read data block failed\n");
        return -1;
    }

    /*
     * 从 old_size 位置开始追加新内容。
     */
    memcopy_fs(data + old_size, content, append_len);

    if (block_write(data_block, data) < 0) 
    {
        printf("fs_appendfile: write data block failed\n");
        return -1;
    }

    /*
     * 更新 inode.size。
     */
    new_size = old_size + append_len;
    dip[inum].size = new_size;

    if (block_write(IBLOCK, buf) < 0) 
    {
        printf("fs_appendfile: write inode table failed\n");
        return -1;
    }

    printf("fs_appendfile: appended %s inum=%d block=%d old=%d append=%d new=%d\n",
           name, inum, data_block, old_size, append_len, new_size);

    return 0;
}

int fs_unlink(const char *name)
{
    unsigned char buf[BSIZE];
    unsigned char ibitmap[BSIZE];
    unsigned char dbitmap[BSIZE];
    struct dinode *dip;
    struct dinode ino;
    struct dinode rootino;
    struct dirent *de;
    int inum;
    int found;
    unsigned int nentry;
    unsigned int data_index;

    /*
     * 不允许删除根目录。
     */
    if (streq_fs(name, "/")) 
    {
        printf("fs_unlink: cannot unlink root\n");
        return -1;
    }

    inum = fs_lookup(name);
    if (inum < 0) 
    {
        printf("fs_unlink: file not found: %s\n", name);
        return -1;
    }

    if ((unsigned int)inum >= NINODES) 
    {
        printf("fs_unlink: bad inum=%d\n", inum);
        return -1;
    }

    /*
     * 读取 inode table。
     */
    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_unlink: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;
    ino = dip[inum];
    rootino = dip[ROOTINO];

    if (ino.type == 0) 
    {
        printf("fs_unlink: inode already free\n");
        return -1;
    }

    if (ino.data_block < DATASTART || ino.data_block >= NBLOCKS) 
    {
        printf("fs_unlink: bad data block=%d\n", ino.data_block);
        return -1;
    }

    /*
     * 清空目标 inode。
     */
    dip[inum].type = 0;
    dip[inum].size = 0;
    dip[inum].data_block = 0;

    if (block_write(IBLOCK, buf) < 0) 
    {
        printf("fs_unlink: write inode table failed\n");
        return -1;
    }

    /*
     * 清空文件数据块。
     */
    memzero_fs(buf, BSIZE);

    if (block_write(ino.data_block, buf) < 0) 
    {
        printf("fs_unlink: clear data block failed\n");
        return -1;
    }

    /*
     * 从 root directory 中删除对应 dirent。
     * 这里采用最简单方式：把该 dirent 清零，不压缩目录。
     */
    memzero_fs(buf, BSIZE);

    if (block_read(ROOTDIR_BLOCK, buf) < 0) 
    {
        printf("fs_unlink: read root directory failed\n");
        return -1;
    }

    de = (struct dirent *)buf;
    nentry = rootino.size / sizeof(struct dirent);
    found = 0;

    for (unsigned int i = 0; i < nentry; i++) 
    {
        if (de[i].inum == (unsigned int)inum && streq_fs(de[i].name, name)) 
        {
            de[i].inum = 0;
            memzero_fs(de[i].name, sizeof(de[i].name));
            found = 1;
            break;
        }
    }

    if (!found) 
    {
        printf("fs_unlink: dirent not found\n");
        return -1;
    }

    if (block_write(ROOTDIR_BLOCK, buf) < 0) 
    {
        printf("fs_unlink: write root directory failed\n");
        return -1;
    }

    /*
     * 更新 inode bitmap。
     */
    memzero_fs(ibitmap, BSIZE);

    if (block_read(IBITMAP_BLOCK, ibitmap) < 0) 
    {
        printf("fs_unlink: read inode bitmap failed\n");
        return -1;
    }

    bitmap_clear(ibitmap, (unsigned int)inum);

    if (block_write(IBITMAP_BLOCK, ibitmap) < 0) 
    {
        printf("fs_unlink: write inode bitmap failed\n");
        return -1;
    }

    /*
     * 更新 data bitmap。
     */
    memzero_fs(dbitmap, BSIZE);

    if (block_read(DBITMAP_BLOCK, dbitmap) < 0) 
    {
        printf("fs_unlink: read data bitmap failed\n");
        return -1;
    }

    data_index = ino.data_block - DATASTART;
    bitmap_clear(dbitmap, data_index);

    if (block_write(DBITMAP_BLOCK, dbitmap) < 0) 
    {
        printf("fs_unlink: write data bitmap failed\n");
        return -1;
    }

    printf("fs_unlink: removed %s inum=%d block=%d\n",
           name, inum, ino.data_block);

    return 0;
}

int fs_open(const char *name)
{
    unsigned char buf[BSIZE];
    struct dinode *dip;
    int inum;

    inum = fs_lookup(name);
    if (inum < 0) 
    {
        printf("fs_open: file not found: %s\n", name);
        return -1;
    }

    if ((unsigned int)inum >= NINODES) 
    {
        printf("fs_open: bad inum=%d\n", inum);
        return -1;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_open: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;

    if (dip[inum].type != T_FILE) 
    {
        printf("fs_open: not a file: %s\n", name);
        return -1;
    }

    return inum;
}

int fs_readi_at(unsigned int inum, char *dst, unsigned int max, unsigned int off)
{
    unsigned char buf[BSIZE];
    struct dinode *dip;
    struct dinode ino;
    unsigned int n;

    if (inum >= NINODES) 
    {
        printf("fs_readi_at: bad inum %d\n", inum);
        return -1;
    }

    if (max == 0) 
    {
        return 0;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(IBLOCK, buf) < 0) 
    {
        printf("fs_readi_at: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)buf;
    ino = dip[inum];

    if (ino.type != T_FILE) 
    {
        printf("fs_readi_at: inode %d is not file\n", inum);
        return -1;
    }

    if (ino.size > BSIZE) 
    {
        printf("fs_readi_at: file too large size=%d\n", ino.size);
        return -1;
    }

    if (off >= ino.size) 
    {
        return 0;
    }

    n = ino.size - off;
    if (n > max) 
    {
        n = max;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(ino.data_block, buf) < 0) 
    {
        printf("fs_readi_at: read data block failed\n");
        return -1;
    }

    memcopy_fs(dst, buf + off, n);

    return n;
}

int fs_writei_at(unsigned int inum, const char *src, unsigned int n, unsigned int off)
{
    unsigned char ibuf[BSIZE];
    unsigned char data[BSIZE];
    struct dinode *dip;
    struct dinode ino;
    unsigned int new_size;

    if (inum >= NINODES) 
    {
        printf("fs_writei_at: bad inum %d\n", inum);
        return -1;
    }

    if (n == 0) 
    {
        return 0;
    }

    if (off >= BSIZE || off + n > BSIZE) 
    {
        printf("fs_writei_at: write too large off=%d n=%d\n", off, n);
        return -1;
    }

    memzero_fs(ibuf, BSIZE);

    if (block_read(IBLOCK, ibuf) < 0) 
    {
        printf("fs_writei_at: read inode table failed\n");
        return -1;
    }

    dip = (struct dinode *)ibuf;
    ino = dip[inum];

    if (ino.type != T_FILE) 
    {
        printf("fs_writei_at: inode %d is not file\n", inum);
        return -1;
    }

    if (ino.data_block < DATASTART || ino.data_block >= NBLOCKS) 
    {
        printf("fs_writei_at: bad data block=%d\n", ino.data_block);
        return -1;
    }

    memzero_fs(data, BSIZE);

    if (block_read(ino.data_block, data) < 0) 
    {
        printf("fs_writei_at: read data block failed\n");
        return -1;
    }

    /*
     * 从 off 位置开始写入 n 字节。
     * 这会覆盖原文件中对应范围的内容。
     */
    memcopy_fs(data + off, src, n);

    if (block_write(ino.data_block, data) < 0) 
    {
        printf("fs_writei_at: write data block failed\n");
        return -1;
    }

    /*
     * 如果写入越过原文件末尾，就扩大 size。
     * 如果只是覆盖中间部分，size 不变。
     */
    new_size = off + n;
    if (new_size > ino.size) 
    {
        dip[inum].size = new_size;

        if (block_write(IBLOCK, ibuf) < 0) 
        {
            printf("fs_writei_at: update inode size failed\n");
            return -1;
        }
    }

    return (int)n;
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

    if (sb.inode_bitmap_start != IBITMAP_BLOCK) 
    {
        printf("fs test failed: bad inode_bitmap_start=%d\n", sb.inode_bitmap_start);
        return;
    }

    if (sb.data_bitmap_start != DBITMAP_BLOCK) 
    {
        printf("fs test failed: bad data_bitmap_start=%d\n", sb.data_bitmap_start);
        return;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(IBITMAP_BLOCK, buf) < 0) 
    {
        printf("fs test failed: read inode bitmap failed\n");
        return;
    }

    if (!bitmap_get(buf, ROOTINO) ||
        !bitmap_get(buf, HELLOINO) ||
        !bitmap_get(buf, READMEINO))
    {
        printf("fs test failed: inode bitmap bad\n");
        return;
    }

    memzero_fs(buf, BSIZE);

    if (block_read(DBITMAP_BLOCK, buf) < 0) 
    {
        printf("fs test failed: read data bitmap failed\n");
        return;
    }

    if (!bitmap_get(buf, ROOTDIR_BLOCK - DATASTART) ||
        !bitmap_get(buf, HELLO_BLOCK - DATASTART) ||
        !bitmap_get(buf, README_BLOCK - DATASTART)) 
    {
        printf("fs test failed: data bitmap bad\n");
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

    printf("fs test passed. size=%d nblocks=%d ninodes=%d root_data=%d read_n=%d bitmap=ok\n",
       sb.size, sb.nblocks, sb.ninodes, rootino.data_block, n);

    printf("fs file content: %s", filebuf);
    printf("fs readme content: %s", readmebuf);

}