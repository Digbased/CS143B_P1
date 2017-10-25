#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "file_system.h"
#include "io_system.h"

#define BITS 8
#define FILES_COUNT 3
#define FREE -1
#define NUM_INTS_PER_BLOCK (L) / 4 //should be 16 integers
#define FD_SIZE 4 //4 integers

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
	for(int i = 0;i < FILES_COUNT;++i)
		files[i] = NULL;
}

void print_bitmap()
{
	char bitmap[B];
	io_system.read_block(0,bitmap,B);
	
	int n = B / BITS;//is the number of bytes required to store the bitmap in block 0
	for(int k = n;k >= 0;--k)
	{
		for(int i = BITS - 1;i >= 0; --i)
		{
			int isEnabled = (bitmap[k] >> i) & (0x01);
			printf("%d",isEnabled);	
		}
	}
	printf("\n");
}

void print_blocks()
{
	for(int k = 0;k < B;++k)
	{
		char block[B];
		io_system.read_block(k,block,B);

		printf("logical_index: %d\n", k);
		for(int i = 0;i < B; i += sizeof(int))
		{
			printf("%d ",*(int*)(&block[i]) );
		}
		printf("\n");
	}

}

//create a new file with the specified name sym_name
static void create(char* filename)
{
	//printf("sizeof(filename): %lu\n",sizeof(*filename));
	//assert(sizeof(filename) <= 4);

	printf("create file: %s\n",filename);

	int fd_index = -1;	
	for(int logical_index = 1;logical_index < 7; ++logical_index)
	{
	//find a free file descriptor in ldisk
		char block[B];
		io_system.read_block(logical_index,block,B);
		int numInts = B / sizeof(int);//16
		int numFDsPerBlock = numInts / FD_SIZE;//4

		for(int k = 0;k < B; k += numFDsPerBlock)
		{
			int fdValue = *(int*)(block + sizeof(int) * k);
			if(fdValue == FREE)
			{
				//mark as taken
				int taken = 0;
				memcpy(&(block[sizeof(int) * k]),&taken,sizeof(int));
				fd_index = (sizeof(int) * k) + (numFDsPerBlock * (logical_index - 1));
				break;
			}
		}
		
		if(fd_index != -1)
			break;		
	}

	//THIS IS ALL HARDCODED RIGHT NOW SINCE I DON'T HAVE CODE THE INITIALIZES THE DIRECTORY
	//find a free directory entry in the directory file and on disk
	char directorymap[B];
	io_system.read_block(1,directorymap,B);
	//first block number as to where to find the data to locate the free directory entry in the file
	int dir_datablock_index = *(int*)(&directorymap[4]);

	char directoryData[B];
	io_system.read_block(dir_datablock_index,directoryData,B);

	//TODO: check if directoryData entry already has valid entry in it... and if it does, iterate to another directory entry until a free one has be found


	//hard code directory filename and its descriptor index for now!
	//directory entry on disk can hold 2 integers
	//1st entry is filename
	//2nd entry is file descriptor index
	memcpy(&(directoryData[0]),filename,sizeof(char) * 4);
	memcpy(&(directoryData[4]),&fd_index,sizeof(fd_index));	

	//when I open the directory file.. I have to make sure I open it as append mode to ensure I don't override previous directory entries if an existing ldisk file was loaded
	//write free directory entry to directory file ~ I don't understand what rewinding means in his instructions
	fprintf(directory_file,"%s %d\n",filename,fd_index);

	//fill both entries ~ write directory data back to ldisk
	io_system.write_block(dir_datablock_index,directoryData, B);
	
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


void init_bitmap()
{
	//first 7 bits of the bitmap are set to 1 since each bit represents a block
	//0th bit represents bitmap
	//1th to 6th bits represents data blocks reserved for file descriptors
	char bitmap[B] = {0};
	for(int i = 0;i < BITS - 1;++i)
		bitmap[0] = bitmap[0] | (0x01 << i);
	io_system.write_block(0,bitmap,B);
	
}

//helper function to initialize the directory when ldisk first boots up
void init_dir()
{
	directory_file = fopen("dir", "a+");
	int dirmap[B / sizeof(int)];
	memset(dirmap,FREE,B);
	//fd0, block 1 is reserved as directory file descriptor with file len initialized to 0 assuming its a new disk
	dirmap[0] = 0;
	io_system.write_block(1,(char*)dirmap,B);
}

void init_disk()
{
	//easiest way to initialize ldisk assuming no filename was specified is to set all blocks in the disk to be free = -1
	for(int logical_index = 0;logical_index < B; ++logical_index)
	{
		int free = -1;
		int block[B / sizeof(int)];//should be 16
		for(int i = 0;i < B / sizeof(int); ++i)
		{
			memcpy(&block[i],&free,sizeof(int));
		}

		io_system.write_block(logical_index,(char*)block,B);			
	}

}

//restores ldisk from file.txt or create a new one if no file exists
//returns 0 if disk is initialized with no filename specified or file does not exist and will be created..
//returns 1 if file does exist
static int init(char* filename)
{
	//init_mask();
	//init_file_pointers();
	
	FILE* ldisk_file = fopen(filename,"r");

	int status = -1;
	//if filename not specified or does not exist
	if(filename == NULL || ldisk_file == NULL)
	{
		status = 0;
		puts("disk initialized");
		init_disk();
		init_bitmap();
		init_dir();
		
	}
	else // if filename exists and successfully loads
	{
		status = 1;
		puts("disk restored");
		fclose(ldisk_file);
		//TODO: load contents of file (ldisk.txt) into ldisk
		// 1. load in bitmap (block 0)
		// 2. load in fds (blocks 1 to 6)
		// 3. load in directory data (ranges from 1 to 3 data blocks)
		// 4. load in data blocks for other files to be loaded on disk

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
