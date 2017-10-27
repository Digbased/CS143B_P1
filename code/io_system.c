#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "io_system.h"

extern char ldisk[L][B];
extern open_file_table oft[OFT_SIZE];

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


//gets file descriptor information (metadata) about the file loaded on disk
file_descriptor GetFD(int block_number,int fd_index)
{
	assert(block_number >= 0 && block_number < LOGICAL_BLOCKS);
	assert(fd_index >= 0 && fd_index < DIR_ENTRIES_PER_BLOCK / 2);//can only hold 4 fds per block

	int buffer[INTS_PER_BLOCK];	
	io_system.read_block(block_number,(char*)buffer);
	
	file_descriptor fd;
	fd.file_len = buffer[fd_index * FD_CAPACITY];
	for(int f = fd_index * FD_CAPACITY,i = 0;i < DISK_BLOCKS_COUNT;++f,++i)
	{
		//logical indices of where the data is actually stored on ldisk
		fd.block_numbers[i] = buffer[f];
	}

	return fd;
}

//moves data on ldisk to an oft entry buffer
//oft_index: the file's buffer location
//block_number: the logical block index on ldisk you want to retrieve data from
void TransferDataToBuffer(int oft_index, int block_number)
{
	assert(oft_index >= 0 && oft_index < OFT_SIZE);
	assert(block_number >= 0 && block_number < LOGICAL_BLOCKS);

	char dataBlock[B];
	io_system.read_block(block_number,dataBlock);
	memcpy(oft[oft_index].buffer,dataBlock,sizeof(dataBlock));
}

//moves open file table buffer contents to ldisk
void TransferBufferToDisk(int oft_index,int block_number)
{
	assert(oft_index >= 0 && oft_index < OFT_SIZE);
	assert(block_number >= 0 && block_number < LOGICAL_BLOCKS);
	io_system.write_block(block_number,oft[oft_index].buffer);
}
