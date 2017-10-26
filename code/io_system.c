#include <stdio.h>
#include <assert.h>
#include "io_system.h"

extern char ldisk[L][B];

//read block copies the logical block ldisk[i] into main memory starting at the location
//specified by the pointer
//size is the size of dest in bytes
static void read_block(int logical_index,char* dest)
{	
	assert(logical_index >= 0 && logical_index < L);
	//assert(logical_index < size);

	//should be able to read both char and int values from ldisk to dest
	int i;	
	for(i = 0;i < B;++i) 
		dest[i] = ldisk[logical_index][i];
	//dest[i] = '\0';
}

//write block copies the number of character corresponding to the block length, B, from main memory
//starting at the location specified by the pointer p, into the logical block ldisk[i].
//returns the logical_index 
static int write_block(int logical_index,char* src)
{
	assert(logical_index >= 0 && logical_index < L);
	//assert(logical_index < size);

	int i;
	for(i = 0;i < B;++i)
		ldisk[logical_index][i] = src[i];
	//ldisk[logical_index][i] = '\0';

	return logical_index;	
}

iospace_struct const io_system = 
{                        
	read_block,        
	write_block     
};
