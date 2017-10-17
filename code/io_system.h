#ifndef IO_SYSTEM_H
#define IO_SYSTEM_H

#define L 64 // logical block size
#define B 64 //block length in bytes


//each block is 16 ints since each block size = 64 bytes
//use this to increment through a block by the size of integer value in bytes
#define I sizeof(int)

/*
	The following class simulates a physical disk where it can be read from and written to
	one sector at a time
*/

typedef struct ldisk
{
	char** buf;
	int logical_block_size;
	int block_length;
	
}ldisk;

ldisk pdisk;


typedef struct
{
	//reads B bytes into dest array
	void (* const read_block)(int logical_index,char* dest);
	//writes a block of memory (src)  into ldisk at logical_index location specified
	//returns 1 on success, 0 on failure
	int (* const write_block)(int logical_index,char* src);
	
	//restores ldisk from file.txt or create a new one if no file exists
	int (* const init)(char* filename);
	
	//frees ldisk dynamic allocations to prevent memory leak
	void (* const free_disk)();	

	//save ldisk to file.txt
	int (* const save)(char* filename);
	
	//returns 0 if block is free, otherwise it returns 1 if block is taken
	int (* const isBitEnabled)(int logical_index);
} iospace_struct;
extern iospace_struct const io_system;


#endif
