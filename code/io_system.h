#ifndef IO_SYSTEM_H
#define IO_SYSTEM_H

#define L 64 // logical block size
#define B 64 //block length in bytes


//each block is 16 ints since each block size = 64 bytes
//use this to increment through a block by the size of integer value in bytes
#define I sizeof(int)

typedef struct
{
	//reads B  bytes into dest array
	//if dest array is smaller than B bytes, then it will read the 
	//first 'size' bytes from ldisk to dest
	void (* const read_block)(int logical_index,char* dest,int size);
	//writes a block of memory (src)  into ldisk at logical_index location specified
	//returns 1 on success, 0 on failure
	int (* const write_block)(int logical_index,char* src,int size);
		
} iospace_struct;
extern iospace_struct const io_system;


#endif
