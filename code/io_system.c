#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "io_system.h"

#define BITS 8

extern ldisk pdisk;

//used to enable / disable bits in bitmap
//masks are ordered from most significant bit first to least significant bit last
static int MASK[BITS];
static int MASK2[BITS];


//initialize 8 bit masks diagonally
void init_mask()
{

	//initialize the bit masks starting from most significant bit to least
	//significant order
	MASK[0] = 0x80;
	MASK2[0] = ~MASK[0];
	for(int i = 1;i < BITS;++i)
	{
		MASK[i] = MASK[i - 1] >> 1;
		MASK2[i] = ~MASK[i];
	}
}

//used to print out bit mask values for debugging purposes
void printMaskValues()
{
	for(int i = 0;i < BITS;++i)
		printf("%d | %04x\n",MASK[i], MASK[i]);
}

//used to print out values in binary form used for debugging
void printBinaryValue()
{
	//print out binary representation of cstring
	char* bitmap = pdisk.buf[0];
	for(int i = 8 - 1;i >= 0;--i)
	{
		for(int t = 8 - 1;t >= 0;--t)
	      	{
	      
	        	int bit = (bitmap[i] >> t) & 0x01;
			printf("%d",bit);
	      	}
	}
	
	printf("\n");
}

void printBinaryMaskValue(int index)
{
	puts("MASK");
	for(int i = 31;i >= 0;--i)
	{
		printf("%d",(MASK[index] >> i) & 0x01);
	}
	printf("\n");
	printf("hi: %d\n",MASK[index]);
	puts("~MASK");
	for(int i = 31;i >= 0;--i)
	{
		printf("%d", (MASK2[index] >> i) & 0x01);
	}
	printf("\n");
}

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

//restores ldisk from file.txt or create a new one if no file exists
//returns 0 if disk is initialized with no filename specified
//returns 1 if file does not exist and is being created as a new file
//returns 2 if file does exist and file contents is loaded into disk
static int init(char* filename)
{

	init_mask();

	pdisk.logical_block_size = L;
	pdisk.block_length = B;

	//allocate and initialize ldisk
	pdisk.buf = (char**)malloc(sizeof(char*) * pdisk.logical_block_size);
	for(int i = 0;i < pdisk.logical_block_size; ++i)
	{
		pdisk.buf[i] = (char*)malloc(sizeof(char) * pdisk.block_length);
		memset(pdisk.buf[i],0,pdisk.block_length);
	}

	//block 0 is reserved for the bitmap ~ assumption: first 2 integers of block 0 are used for bitmap
	//since bits are read from right to left block 0 resides in the second half of the bitmap
	pdisk.buf[0][0] = 1; // this is a char array not an integer array
	
	int return_status = -1;
	if(filename == NULL)
	{	
		//printf("disk initialized\n");
		return_status = 0;	
	}
	else
	{
		//haven't tested this yet... will need to later 
		FILE* file = fopen(filename,"r");
		if(file == NULL)
		{
			//create new file
			printf("Create new file: %s\n, since file did not exist\n",filename);
			file = fopen(filename,"a+");
			return_status = 1;	
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
			return_status = 2;		
		}

		fclose(file);
	}

	return return_status;
}

//frees the pdisk to prevent memory leaks
static void free_disk()
{
	for(int i = 0;i < pdisk.logical_block_size;++i)
		free(pdisk.buf[i]);
	free(pdisk.buf);
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
	assert(logical_index >= 0 && logical_index < pdisk.logical_block_size);

	char* bitmap = pdisk.buf[0];

	//since each byte is 8 bits and ldisk is a 2d array of characters... the first 8 character bytes in block 0 of the array refer to the disk's bitmap
	//we have take the modulus here to ensure it grabs the correct bit of the correct index
	
	int bucket_index = logical_index / BITS; // describes which byte in the bitmap array we are referencing	
	int bit_index = logical_index % BITS; // describes which bit in the bitmap array indexed by bucket_index  we are referencing
	int bitEnabled = (bitmap[bucket_index] >> bit_index) & 1;
	//printf("is block [%d] taken ? %d\n",logical_index,bitEnabled);	

	return bitEnabled;
}

static void enableBit(int logical_index)
{
	assert(logical_index >= 0 && logical_index < pdisk.logical_block_size);	
	
	char* bitmap = pdisk.buf[0];

	//int bucket_length = pdisk.logical_block_size / BITS;
	int bucket_index = logical_index / BITS;
	int bit_index = logical_index % BITS;

	//reminder: the bit mask is initialized from  msb to lsb order
	bitmap[bucket_index] = bitmap[bucket_index] | MASK[BITS - 1 - bit_index];
		
}

static void disableBit(int logical_index)
{
	assert(logical_index >= 0 && logical_index < pdisk.logical_block_size);
	
	char* bitmap = pdisk.buf[0];
	int bucket_index = logical_index / BITS;
	int bit_index = logical_index % BITS;
	bitmap[bucket_index] = bitmap[bucket_index] & (MASK2[BITS - 1 - bit_index]);

	//printf("MASK[%d] = %04x\n",bit_index,MASK[bit_index]);
	//printf("MASK2[%d] = %04x\n",bit_index,MASK2[bit_index]);

	//printBinaryValue();
	//printBinaryMaskValue(bit_index);

	//printMaskValues();
}                        

iospace_struct const io_system = 
{                        
	read_block,        
	write_block,       
	init,              
	free_disk,         
	save,              
	isBitEnabled,      
	enableBit,
	disableBit
};
