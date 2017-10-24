#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "file_system.h"
#include "io_system.h"

#define BITS 8
#define FILES_COUNT 3
#define FREE -1

extern char ldisk[L][B];


//8 BITS used to set and clear bits in the bitmap of block 0
static int MASK[BITS]; 

FILE* directory_file;
FILE* files[FILES_COUNT];

//helper function to initialize bit masks
//initialize 8 bit masks diagonally
void init_mask()
{
	//initialize the bit masks starting from most significant bit to least
	//significant order
	MASK[0] = 0x80;
	for(int i = 1;i < BITS;++i)
	{
		MASK[i] = MASK[i - 1] >> 1;
	}
}


void init_file_pointers()
{
	directory_file = NULL;
//	for(int i = 0;i < FILES_COUNT;++i)
//		files[i] = NULL;
}


//create a new file with the specified name sym_name
static void create(char* filename)
{
	printf("create file: %s\n",filename);
}

//destroy the named file
//returns 1 on success, 0 on error
static int destroy(char* filename)
{
	printf("destroy file: %s\n",filename);
	return 0;
}

//open the named file for reading and writing
//returns an index value which is used by subsequent read, write, lseek, or close operations (OFT index)
static int open(char* filename)
{
	printf("open file: %s\n",filename);
	return -1;
}

//closes the specified file
static void close(int index)
{
	printf("close file at index: %d\n",index);
}

// -- operations available after opening the file --//



//returns the number of successfully read bytes from file into main memory
//reading begins at the current position of the file
static int read(int index,char* mem_area,int count)
{
	printf("begin reading %d bytes from open file to mem_area\n",count);
	return -1;
}

//returns number of bytes written from main memory starting at mem_area into the specified file
//writing begins at the current position of the file
static int write(int index,char* mem_area,int count)
{
	printf("begin writing %d bytes from mem_area to open file\n",count);
	return -1;
}

//move to the current position of the file to pos, where pos is an integer specifying
//the number of bytes from the beginning of the file. When a file is initially opened,
//the current position is automatically set to zero. After each read or write
//operation, it points to the byte immediately following the one that was accessed last.
static void lseek(int index, int pos)
{
	printf("lseek to pos: %d at file index: %d\n",pos,index);
}

//displays list of files and their lengths
static void directory()
{
	printf("print out the current directory here\n");
}

//restores ldisk from file.txt or create a new one if no file exists
//returns 0 if disk is initialized with no filename specified
//returns 1 if file does not exist and is being created as a new file
//returns 2 if file does exist and file contents is loaded into disk
static int init(char* filename)
{
	init_mask();
	init_file_pointers();

	//first 7 bits of the bitmap are set to 1 since each bit represents a block
	//0th bit represents bitmap
	char bitmap[B] = {0};
	io_system.read_block(0,bitmap,64);
	for(int i = 0;i < BITS - 1;++i)
		bitmap[0] = bitmap[0] | (0x01 << i);
	io_system.write_block(0,bitmap,64);

	//open a file named directory.txt and have it initialized in fd0
	directory_file = fopen("directory.txt","w+");

	char directorymap[B] = {0};
	//directory file descriptor
	int dir_desc[4] = {0, -1, -1, -1};
	for(int i = 0;i < 4;++i)
	{
		//copy 4 bytes from char array  since this is treated as an int
		char* directoryAddress = directorymap + 4 * i;
		int* directoryValueLocation = &(dir_desc[i]);
		memcpy(directoryAddress,directoryValueLocation,sizeof(int));
	}

	//for all other file descriptors in block 1, set them as free fds
	int free_fd = -1;
	for(int offset = 16;offset < B;offset += 4)
	{
		char* directoryAddress = directorymap + offset;
		int* directoryValueLocation = &free_fd;
		memcpy(directoryAddress,directoryValueLocation,sizeof(int));	
	}

	//write back to block1
	io_system.write_block(1,directorymap,64);

	//for blocks 2 - 6, init all of them to be free file descriptors
#define FD_BLOCK_COUNT  5
	char fd_blocks[FD_BLOCK_COUNT][B];

	for(int i = 0;i < FD_BLOCK_COUNT; ++i)
	{
		for(int offset = 0;offset < B;offset += sizeof(int))
		{
			char* directoryAddress = fd_blocks[i] + offset;
			int* directoryValueLocation = &free_fd;
			memcpy(directoryAddress, directoryValueLocation,sizeof(int));
		}
	}

	//write back to blocks 2 - 6
	for(int i = 2;i < 7;++i)
		io_system.write_block(i,fd_blocks[i - 2],B);
	

	int status = -1;
	if(filename == NULL)
	{
		//puts("disk initialized");
		status = 0;
	}
	else
	{
		//puts("disk restored");
		status = 1;
	}

	return status;
}

//closes ldisk and saves all files opened (including directory file) to some file.txt 
static int save(char* filename)
{
	if(directory_file != NULL)
		fclose(directory_file);
	//for(int i = 0;i < FILES_COUNT; ++i)
	//{
	//	if(files[i] != NULL)
	//		fclose(files[i]);
	//}
	return -1;
}


//returns 0 if block is free, otherwise it returns 1 if block is taken
static int isBitEnabled(int logical_index)
{
	assert(logical_index >= 0 && logical_index < L);

	char bitmap[B] = {0};
	io_system.read_block(0,bitmap,64);

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
	assert(logical_index >= 0 && logical_index < L);
	
	char bitmap[B] = {0};
	io_system.read_block(0,bitmap,64);

	int bucket_index = logical_index / BITS;
	int bit_index = logical_index % BITS;

	//reminder: the bit mask is initialized from  msb to lsb order
	bitmap[bucket_index] = bitmap[bucket_index] | MASK[BITS - 1 - bit_index];
	io_system.write_block(0,bitmap,64);	
}

static void disableBit(int logical_index)
{
	assert(logical_index >= 0 && logical_index < L);

	char bitmap[B] = {0};
	io_system.read_block(0,bitmap,64);

	int bucket_index = logical_index / BITS;
	int bit_index = logical_index % BITS;
	bitmap[bucket_index] = bitmap[bucket_index] & (~MASK[BITS - 1 - bit_index]);
	io_system.write_block(0,bitmap,64);
}                        

filespace_struct const file_system = 
{
	create,
	destroy,
	open,
	close,
	read,
	write,
	lseek,
	directory,
	init,
	save,
	isBitEnabled,
	enableBit,
	disableBit
};
