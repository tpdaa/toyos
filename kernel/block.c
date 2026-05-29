#include "block.h"
#include "printf.h"

static unsigned char fake_disk[NBLOCKS][BSIZE];

static void memzero(void *dst, unsigned int n)
{
    unsigned char *p = (unsigned char *)dst;

    for (unsigned int i = 0; i < n; i++) 
    {
        p[i] = 0;
    }
}

static void memcopy(void *dst, const void *src, unsigned int n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    for (unsigned int i = 0; i < n; i++) 
    {
        d[i] = s[i];
    }
}

void block_init(void)
{
    memzero(fake_disk, NBLOCKS * BSIZE);

    printf("[BLOCK] init blocks=%d bsize=%d\n",
           NBLOCKS, BSIZE);
}

int block_read(unsigned int blockno, void *dst)
{
    if (blockno >= NBLOCKS) 
    {
        printf("block_read: bad blockno %d\n", blockno);
        return -1;
    }

    memcopy(dst, fake_disk[blockno], BSIZE);
    return 0;
}

int block_write(unsigned int blockno, const void *src)
{
    if (blockno >= NBLOCKS) 
    {
        printf("block_write: bad blockno %d\n", blockno);
        return -1;
    }

    memcopy(fake_disk[blockno], src, BSIZE);
    return 0;
}

void block_test(void)
{
    unsigned char wbuf[BSIZE];
    unsigned char rbuf[BSIZE];

    for (unsigned int i = 0; i < BSIZE; i++) 
    {
        wbuf[i] = (unsigned char)(i & 0xff);
        rbuf[i] = 0;
    }

    if (block_write(1, wbuf) < 0) 
    {
        printf("[TEST][FAIL] block write failed\n");
        return;
    }

    if (block_read(1, rbuf) < 0) 
    {
        printf("[TEST][FAIL] block read failed\n");
        return;
    }

    for (unsigned int i = 0; i < BSIZE; i++) 
    {
        if (wbuf[i] != rbuf[i]) 
        {
            printf("[TEST][FAIL] block mismatch at %d\n", i);
            return;
        }
    }

    printf("[TEST] block ok\n");
}
