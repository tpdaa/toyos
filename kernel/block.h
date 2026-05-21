#ifndef BLOCK_H
#define BLOCK_H

#define BSIZE 512
#define NBLOCKS 1024

void block_init(void);
int block_read(unsigned int blockno, void *dst);
int block_write(unsigned int blockno, const void *src);
void block_test(void);

#endif