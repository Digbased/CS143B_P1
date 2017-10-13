#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "io_system.h"

#define BITS 8
#define INT_SIZE sizeof(int) * BITS

extern ldisk pdisk;

//read block copies the logical block ldisk[i] into main memory starting at the location
//specified by the pointer
static void read_block(int logical_index,char* dest)
{
	assert(pdisk.logical_block_size > 0);	
	assert(pdisk.block_length > 0);
	printf("read_block at logical_index: %d\n",logical_index);
	assert(logical_index >=  0 && logical_index < pdisk.logical_block_size);

	int i;	
	for(i = 0;i < pdisk.block_length;++i)
		dest[i] = pdisk.buf[logical_index][i];
	dest[i] = '\0';
}

//write block copies the number of character corresponding to the block length, B, from main memory
//starting at the location specified by the pointer p, into the logical block ldisk[i].
static int write_block(int logical_index,char* src)
{
	assert(pdisk.logical_block_size > 0);	
	assert(pdisk.block_length > 0);
	printf("write_block to ldisk at logical_index: %d\n",logical_index);
	assert(logical_index >= 0 && logical_index < pdisk.logical_block_size);	
	
	//failure block 0 shouldn't be written to since it holds the bitmap
	if(logical_index == 0)
		return 0;

	int i;
	for(i = 0;i < pdisk.block_length;++i)
		pdisk.buf[logical_index][i] = src[i];
	pdisk.buf[logical_index][i] = '\0';
	return 1;	
}

//restores ldisk from file.txt or create a new one if no file exists
static int init(char* filename)
{

	pdisk.logical_block_size = L;
	pdisk.block_length = B;

	//allocate and initialize ldisk
	pdisk.buf = (char**)malloc(sizeof(char*) * pdisk.logical_block_size);
	for(int i = 0;i < pdisk.logical_block_size; ++i)
	{
		pdisk.buf[i] = (char*)malloc(sizeof(char) * pdisk.block_length);
		memset(pdisk.buf[i],0,pdisk.block_length);
	}

	//block 0 is reserved for the bitmap
	pdisk.buf[0][0] = 1;

	if(filename == NULL)
	{	
		printf("disk initialized\n");
		
	}
	else
	{
		FILE* file = fopen(filename,"r");
		if(file == NULL)
		{
			//create new file
			printf("Create new file: %s\n, since file did not exist\n",filename);
			file = fopen(filename,"a+");	
		}
		else
		{
			//restore ldisk from open file
			printf("Restoring disk from file: %s\n",filename);
			for(int i = 0;i < pdisk.logical_block_size;++i)
			{
				fgets(pdisk.buf[i],pdisk.block_length,file);
				//fread(pdisk.buf[i],sizeof(char),pdisk.block_length,file); 
				//pdisk.buf[i][pdisk.block_length] = '\0';
			}		
		}

		fclose(file);
	}

	return 0;
}
//save ldisk to file.txt
static int save(char* filename)
{
	return -1;
}

//returns 0 if block is free, otherwise it returns 1 if block is taken
static int isBitEnabled(int logical_index)
{
	assert(pdisk.logical_block_size > 0);	
	assert(pdisk.block_length > 0);
	assert(logical_index > 0 && logical_index < pdisk.logical_block_size);

	char* bm = pdisk.buf[0];

	int bitEnabled = (*bm >> logical_index) & 1;
	printf("is block [%d] taken ? %d\n",logical_index,bitEnabled);	

	return bitEnabled;
}

iospace_struct const io_system = 
{
	read_block,
	write_block,
	init,
	save,
	isBitEnabled
};
