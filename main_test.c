#include <stdio.h>
#include <stdlib.h>

#include "code/io_system.h"
#include "code/ldisk.h"

extern ldisk pdisk;

int main(int argc,char* argv[])
{
	io_system.init(NULL);
	
	int number;
	io_system.read_block(0,(char*)&number,sizeof(number));
	printf("number: %d\n", number);

	for(int i = 0;i < 7;++i)
		pdisk.buf[0][0] = pdisk.buf[0][0] | (0x01 << i);

	//I expect this to print out 127
	printf("value: %d\n",(int)pdisk.buf[0][0]);	

	io_system.free_disk();
	return 0;
}
