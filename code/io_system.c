#include <stdio.h>
#include <assert.h>
#include "ldisk.h"
#include "io_system.h"

extern ldisk pdisk;

//read block copies the logical block ldisk[i] into main memory starting at the location
//specified by the pointer
//size is the size of dest in bytes
static void read_block(int logical_index,char* dest,int size)
{
	assert(pdisk.logical_block_size > 0);
	assert(pdisk.block_length > 0);
	printf("read_block at logical_index: %d\n",logical_index);
	assert(logical_index >=  0 && logical_index < pdisk.logical_block_size);
	assert(pdisk.logical_block_size > size);

	//should be able to read both char and int values from ldisk to dest
	int i;	
	for(i = 0;i < size;++i) 
		dest[i] = pdisk.buf[logical_index][i];
	//dest[i] = '\0';
}

//write block copies the number of character corresponding to the block length, B, from main memory
//starting at the location specified by the pointer p, into the logical block ldisk[i].
static int write_block(int logical_index,char* src,int size)
{
	assert(pdisk.logical_block_size > 0);	
	assert(pdisk.block_length > 0);
	//printf("write_block to ldisk at logical_index: %d\n",logical_index);
	assert(logical_index >= 0 && logical_index < pdisk.logical_block_size);	
	assert(pdisk.logical_block_size > size);

	//failure block 0 shouldn't be written to since it holds the bitmap
	if(logical_index == 0)
		return 0;

	int i;
	for(i = 0;i < size;++i)
		pdisk.buf[logical_index][i] = src[i];
	pdisk.buf[logical_index][i] = '\0';
	
	return 1;	
}

iospace_struct const io_system = 
{                        
	read_block,        
	write_block     
};
