#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "io_system.h"

#define FREE -1

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


int GetBlockNumber(int fd_index)
{
	assert(fd_index >= 0 && fd_index < MAX_DIR_ENTRIES);
	int fd_padding = sizeof(int) * FD_CAPACITY;//16 bytes padding
	return ((fd_index * fd_padding) + BYTES_PER_BLOCK) / LOGICAL_BLOCKS;
}

//gets file descriptor information (metadata) about the file loaded on disk
file_descriptor GetFD(int fd_index)
{
	int block_number = GetBlockNumber(fd_index);
	assert(block_number >= 0 && block_number < LOGICAL_BLOCKS);

	int buffer[INTS_PER_BLOCK];	
	io_system.read_block(block_number,(char*)buffer);
	
	file_descriptor fd;
	fd.file_len = buffer[fd_index * FD_CAPACITY];
	for(int f = (fd_index * FD_CAPACITY) + 1,i = 0;i < DISK_BLOCKS_COUNT;++f,++i)
	{
		//logical indices of where the data is actually stored on ldisk
		fd.block_numbers[i] = buffer[f];
	}

	return fd;
}

//the issue with this function is that we don't know ahead of time how many directory entries there are on ldisk
void PrintDirEntries()
{
	file_descriptor file_dir = GetFD(0);

	for(int i = 0;i < DISK_BLOCKS_COUNT;++i)
	{
		//if the next data block is unassigned.. stop 
		if(file_dir.block_numbers[i] == FREE) break;
	
		printf("data block: %d\n",file_dir.block_numbers[i]);
		char dirContents[B];
		io_system.read_block(file_dir.block_numbers[i],dirContents);
		int dir_entry_size = DIR_ENTRY_CAPACITY * sizeof(int);//size of each dir entry in bytes
		for(int k = 0;k < B;k += dir_entry_size)
		{
			int status = *(int*)(&dirContents[k]);
			if(status == FREE) break;
			char* sym_name = (char*)(&dirContents[k]);
			int fd_index = *(int*)(&dirContents[k + sizeof(int)]);
			printf("%d. %s %d\n",k,sym_name,fd_index);			
		}
		printf("\n");		
	}
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
