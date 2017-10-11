#ifndef IO_SYSTEM_H
#define IO_SYSTEM_H

#define L 64 // logical block size
#define B 64 //block length in bytes

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

//reads B bytes into dest array
void read_block(int logical_index,char* dest);
//writes a block of memory (src)  into ldisk at logical_index location specified
//returns 1 on success, 0 on failure
int write_block(int logical_index,char* src);

//restores ldisk from file.txt or create a new one if no file exists
int init(char* filename);
//save ldisk to file.txt
int save(char* filename);


#endif
