#include <stdio.h>
#include <assert.h>
#include "io_system.h"

extern char ldisk[L][B];

//read block copies the logical block ldisk[i] into main memory starting at the location
//specified by the pointer
//size is the size of dest in bytes
static void read_block(int logical_index,char* dest,int size)
{	
	assert(logical_index >= 0 && logical_index < L);
	//assert(logical_index < size);

	//should be able to read both char and int values from ldisk to dest
	int i;	
	for(i = 0;i < size;++i) 
		dest[i] = ldisk[logical_index][i];
		//dest[i] = pdisk.buf[logical_index][i];
	//dest[i] = '\0';
}

//write block copies the number of character corresponding to the block length, B, from main memory
//starting at the location specified by the pointer p, into the logical block ldisk[i].
static int write_block(int logical_index,char* src,int size)
{
	assert(logical_index >= 0 && logical_index < L);
	//assert(logical_index < size);

	//failure blocks 0 to 6  shouldn't be written to since it holds the bitmap and fds
//	if(logical_index < 7)
//		return 0;

	int i;
	for(i = 0;i < size;++i)
		ldisk[logical_index][i] = src[i];
	//ldisk[logical_index][i] = '\0';

	return 1;	
}

iospace_struct const io_system = 
{                        
	read_block,        
	write_block     
};
