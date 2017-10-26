#ifndef TEST_SPECS_H
#define TEST_SPECS_H

#define LOGICAL_BLOCKS 64
#define BYTES_PER_BLOCK 64

//capacity of each file descriptor measured in integers
#define FD_CAPACITY 4

#define INTS_PER_BLOCK ((BYTES_PER_BLOCK) / sizeof(int)) //16 integers
//file descriptors per block
#define FDS_PER_BLOCK ((INTS_PER_BLOCK) / (FD_CAPACITY))

//capacity of each directory entry measured in integers
#define DIR_ENTRY_CAPACITY 2

//max number of data blocks directory can allocate on ldisk
#define DIR_BLOCKS 3

//max number of directory entries held in ldisk ~ 24 entries = 24 descriptors
#define MAX_DIR_ENTRIES ((DIR_BLOCKS) * (INTS_PER_BLOCK) / (DIR_ENTRY_CAPACITY)) 

//number of blocks reserved for bitmap
#define RESERVED_BITMAP_BLOCKS 1

//number of reserved file descriptor blocks on ldisk ~ 24 
#define RESERVED_FD_BLOCKS (((MAX_DIR_ENTRIES) * (FD_CAPACITY)) / (INTS_PER_BLOCK))

//total number of reserved blocks on ldisk ~ 7
#define RESERVED_BLOCKS ((RESERVED_BITMAP_BLOCKS) + (RESERVED_FD_BLOCKS))

//number of elements in open file table (OFT) ~ 4 (directory + 3 open files)
#define OFT_SIZE 4

#endif //TEST_SPECS_H
