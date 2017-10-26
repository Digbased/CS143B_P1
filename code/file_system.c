#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "file_system.h"
#include "io_system.h"

#define BITS 8
#define FILES_COUNT ((OFT_SIZE) - 1)
//free refers to a free fd or unassigned block number in a fd
#define FREE -1

extern char ldisk[L][B];


//8 BITS used to set and clear bits in the bitmap of block 0
static int MASK[BITS]; 

//files available in the OFT ~ might make into static later on
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
	io_system.read_block(0,bitmap);
	
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
		io_system.read_block(k,block);

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
	printf("create file: %s\n",filename);

	int fd_index = -1;	
	for(int logical_index = 1;logical_index < 7; ++logical_index)
	{
	//find a free file descriptor in ldisk
		char block[B];
		io_system.read_block(logical_index,block);
		
		for(int k = 0;k < B; k += FDS_PER_BLOCK)
		{
			int fdValue = *(int*)(block + sizeof(int) * k);
			if(fdValue == FREE)
			{
				//mark as taken
				//int taken = 0;
				//memcpy(&(block[sizeof(int) * k]),&taken,sizeof(int));
				fd_index = (sizeof(int) * k) + (FDS_PER_BLOCK * (logical_index - 1));
				break;
			}
		}
		
		if(fd_index != -1) //file descriptor is found
			break;
		if(fd_index == -1 && logical_index == 6)
		{
			printf("Error: not enough space to store %s on ldisk\n",filename);
			return;
		}
	}

	char directorymap[B];
	io_system.read_block(1,directorymap);

	int directory_filelen = *(int*)(&directorymap[0]);
	int dir_indices[DISK_BLOCKS_COUNT];
	for(int i = 0;i < DISK_BLOCKS_COUNT;++i)
	{
		dir_indices[i] = *(int*)(&directorymap[ (i + 1) * sizeof(int)]);
	}


	//TODO: check if directory entry already exists in file... if it does return error
	char dir_entry[4];
	int dir_entry_fd;
	rewind(directory_file);
	while(fscanf(directory_file,"%s %d",dir_entry,&dir_entry_fd) == DIR_ENTRY_CAPACITY)
	{
		if(strcmp(filename,dir_entry) == 0)
		{
			printf("error: %s already exists in directory file\n",filename);
			return;//-1
		}
	}

	//when create function is invoked, does it promise to keep an updated list of directory entries in both on ldisk and on the file?

	//check if directory entry exists on ldisk
	for(int i = 0;i < DISK_BLOCKS_COUNT;++i)
	{
		if(dir_indices[i] == FREE) //then we know that directory entry does not exist since a datablock has not been assigned for the directory fd
			break;
		else
		{
			char directoryData[B];
			io_system.read_block(dir_indices[i],directoryData);

			int dir_entry_capacity = B / sizeof(int) / 2;
			int dir_entry_len = sizeof(int) * 2;
			for(int k = 0;k < dir_entry_capacity; ++k)
			{
				char* entry_name = (char*)(&directoryData[k * dir_entry_len]);
				if(strcmp(entry_name,filename) == 0)
				{
					printf("error: %s already exists in the directory ldisk\n",filename);
					return;//-1
				}
			}
		}
	}


	for(int i = 0;i < DISK_BLOCKS_COUNT;++i)
	{
		//if dir_index isn't allocated to a data block,find a data block to assign it to in ldisk
		//find free data block directory can use to store its directory entries on ldisk
		if(dir_indices[i] == FREE)
		{
			//hardcode next min free block to be index 7
			for(int logical_index = 7; logical_index < B; ++logical_index)
			{
				if(file_system.isBitEnabled(logical_index) == 0)
				{
					dir_indices[i] = logical_index;//assign block_number to an open datablock on ldisk for the directory fd
					memcpy( &directorymap[ (i + 1) * sizeof(int) ], &logical_index,sizeof(int));
					file_system.enableBit(logical_index);
					break;
				}
			}	
		}
	
		//find a location on ldisk to store directory entry
		int block_number = dir_indices[i];
		char directoryData[B];
		io_system.read_block(block_number,directoryData);
		//TODO: REPLACE DIR_ENTRY_CAPACITY WITH #DEFINE
		int dir_entry_capacity = B / sizeof(int) / 2; //8 directory entries per data block
		int dir_entry_len = sizeof(int) * 2;//how many integers each dir entry can hold in bytes
		for(int k = 0;k < dir_entry_capacity; ++k)
		{
			//if file name is not assigned (-1), then this entry slot is free
			int entry_status = *(int*)(&directoryData[k * dir_entry_len]);
			if(entry_status == FREE)
			{
				//1st entry is filename, 2nd entry is fd index
				memcpy(&(directoryData[k * dir_entry_len]),filename,sizeof(char) * 4);
				memcpy(&(directoryData[k * dir_entry_len + sizeof(int)]),&fd_index,sizeof(fd_index));
				//if the directory entry does not exist, write it to the directory file
				fprintf(directory_file,"%s %d\n",filename,fd_index);

				//printf("directory data entry: %s %d\n",(char*)&directoryData[k * dir_entry_len],*(int*)&directoryData[k * dir_entry_len + sizeof(int)]);

				//update directory file length
				directory_filelen = ftell(directory_file); 
				memcpy(&directorymap[0],&directory_filelen,sizeof(int));

				//printf("directory file length: %d\n",directory_filelen);

				//fill both entries ~ write directory data back to ldisk
				io_system.write_block(block_number,directoryData);

				i = DISK_BLOCKS_COUNT;//do this to stop outer loop
				break;
			}
		}
		

	}

	//write back to directory fd to update ldisk
	io_system.write_block(1,directorymap);
	
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
	io_system.write_block(0,bitmap);
	
}

//helper function to initialize the directory when ldisk first boots up
void init_dir()
{
	directory_file = fopen("directory.txt", "w+");
	int dirmap[B / sizeof(int)];
	memset(dirmap,FREE,B);
	//fd0, block 1 is reserved as directory file descriptor with file len initialized to 0 assuming its a new disk
	dirmap[0] = 0;
	io_system.write_block(1,(char*)dirmap);
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

		io_system.write_block(logical_index,(char*)block);			
	}

}

//restores ldisk from file.txt or create a new one if no file exists
//returns 0 if disk is initialized with no filename specified or file does not exist and will be created..
//returns 1 if file does exist
static int init(char* filename)
{
	init_mask();
	init_file_pointers();

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
		printf("disk restored from file: %s\n",filename);
		//load contents of file (ldisk.txt) into ldisk
		for(int l = 0;l < L;++l)
		{
			char data[B];
			fread((char*)data,sizeof(char),B,ldisk_file);
			io_system.write_block(l,data);
		}

		//restore directory file contents:
		directory_file = fopen("directory.txt","w+");
		if(directory_file == NULL)
		{
			printf("Error: cannot open file directory.txt\n");
			return -1;
		}

		int dirmap[B / sizeof(int)];
		io_system.read_block(1,(char*)dirmap);
		//locate on disk where each datablock is in the directory fd to restore directory file contents
		for(int i = 1;i < FD_CAPACITY;++i)
		{
			//directory fd has no more file content information stored in its disk map
			if(dirmap[i] == FREE) break;

			char directoryData[B];
			io_system.read_block(dirmap[i],directoryData);
			
			int entryBytes = sizeof(int) * 2;//2 integers = 8 bytes
			int numEntriesPerBlock = B / sizeof(int) / 2;
			for(int k = 0;k < numEntriesPerBlock; ++k)
			{
				char* entry_name = (char*)(&directoryData[k * sizeof(char) * entryBytes]);
				int entry_fd = *(int*)(&directoryData[k * entryBytes + sizeof(int)]);
				
				//don't output junk to directory file
				if(entry_fd == FREE) break;

				fprintf(directory_file,"%s %d\n",entry_name,entry_fd);
			}	
		}
	

		//TODO: Also restore each file loaded from ldisk

		fclose(ldisk_file);

	}

	return status;
}

//closes ldisk and saves all files opened (including directory file) to some file.txt 
static int save(char* filename)
{
	if(directory_file != NULL)
		fclose(directory_file);
	for(int i = 0;i < FILES_COUNT; ++i)
	{
		if(files[i] != NULL)
			fclose(files[i]);
	}

	//save all contents stored in ldisk to a file
	FILE* ldisk_file = fopen(filename,"w");
	if(ldisk_file == NULL)
	{
		printf("Error: %s could not be saved\n",filename);
		return -1;
	}

	for(int l = 0;l < L;++l)
	{
		char data[B];
		io_system.read_block(l,data);
		fwrite(data,sizeof(char),sizeof(data),ldisk_file);	
	}

	fclose(ldisk_file);
	return 0;
}


//returns 0 if block is free, otherwise it returns 1 if block is taken
static int isBitEnabled(int logical_index)
{
	assert(logical_index >= 0 && logical_index < L);

	char bitmap[B] = {0};
	io_system.read_block(0,bitmap);

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
	
	char bitmap[B];
	io_system.read_block(0,bitmap);

	int bucket_index = logical_index / BITS;
	int bit_index = logical_index % BITS;

	//reminder: the bit mask is initialized from  msb to lsb order
	bitmap[bucket_index] = bitmap[bucket_index] | MASK[BITS - 1 - bit_index];
	io_system.write_block(0,bitmap);	
}

static void disableBit(int logical_index)
{
	assert(logical_index >= 0 && logical_index < L);

	char bitmap[B];
	io_system.read_block(0,bitmap);

	int bucket_index = logical_index / BITS;
	int bit_index = logical_index % BITS;
	bitmap[bucket_index] = bitmap[bucket_index] & (~MASK[BITS - 1 - bit_index]);
	io_system.write_block(0,bitmap);
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
