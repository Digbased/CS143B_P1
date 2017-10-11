#include <stdio.h>
#include <assert.h>
#include "io_system.h"

//read block copies the logical block ldisk[i] into main memory starting at the location
//specified by the pointer
void read_block(int logical_index,char* dest)
{
	printf("read_block at logical_index: %d\n",logical_index);
	assert(logical_index >=  0 && logical_index < pdisk.logical_block_size);
	
	for(int i = 0;i < pdisk.block_length;++i)
		dest[i] = pdisk.buf[logical_index][i];

}

//write block copies the number of character corresponding to the block length, B, from main memory
//starting at the location specified by the pointer p, into the logical block ldisk[i].
int  write_block(int logical_index,char* src)
{
	printf("write_block to ldisk at logical_index: %d\n",logical_index);
	assert(logical_index >= 0 && logical_index < pdisk.logical_block_size);	
	
	//failure block 0 shouldn't be written to since it holds the bitmap
	if(logical_index == 0)
		return 0;

	for(int i = 0;i < pdisk.block_length;++i)
		pdisk.buf[logical_index][i] = src[i];
	return 1;	
}

//restores ldisk from file.txt or create a new one if no file exists
int init(char* filename)
{
	return -1;
}
//save ldisk to file.txt
int save(char* filename)
{
	return -1;
}

