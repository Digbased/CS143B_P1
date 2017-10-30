#ifndef IO_SYSTEM_H
#define IO_SYSTEM_H

#include "test_specs.h"

#define L (LOGICAL_BLOCKS) // logical block size 64
#define B (BYTES_PER_BLOCK) //block length in bytes 64

//the simulated hard drive disk
char ldisk[L][B];

//open file table (oft)
typedef struct
{
	char buffer[B];
	int cur_pos;//current_position
	int fd_index;//file descriptor index
	int file_len;

}open_file_table;

open_file_table oft[OFT_SIZE];

typedef struct
{
	int file_len;
	int block_numbers[DISK_BLOCKS_COUNT];
}file_descriptor;

typedef struct
{
	char sym_name[4];//filename
	int fd_index;	
}directory_entry;

//io_system struct
typedef struct
{
	//reads B  bytes into dest array
	//if dest array is smaller than B bytes, then it will read the 
	//first 'size' bytes from ldisk to dest
	void (* const read_block)(int logical_index,char* dest);
	//writes a block of memory (src)  into ldisk at logical_index location specified
	//returns 1 on success, 0 on failure
	int (* const write_block)(int logical_index,char* src);
		
} iospace_struct;
extern iospace_struct const io_system;

//helper functions--- might move to io_system
int GetBlockNumber(int fd_index);
file_descriptor GetFD(int fd_index);
void WriteFDToLDisk(int fd_index,file_descriptor fd);

void PrintDirEntries();

//transfers data on disk located at block_number  to one of the open file table entries oft_index
void TransferDataToBuffer(int oft_index, int block_number);

void TransferBufferToDisk(int oft_index,int block_number);
#endif
