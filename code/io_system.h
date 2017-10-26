#ifndef IO_SYSTEM_H
#define IO_SYSTEM_H

#include "test_specs.h"

#define L (LOGICAL_BLOCKS) // logical block size 64
#define B (BYTES_PER_BLOCK) //block length in bytes 64

//the simulated hard drive disk
char ldisk[L][B];

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


#endif
