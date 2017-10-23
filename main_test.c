#include <stdio.h>
#include <stdlib.h>

#include "code/io_system.h"

int main(int argc,char* argv[])
{
	io_system.init(NULL);
	
	int number;
	io_system.read_block(0,(char*)&number,sizeof(number));
	printf("number: %d\n", number);


	io_system.free_disk();
	return 0;
}
