#ifndef LDISK_H
#define LDISK_H

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

#endif
