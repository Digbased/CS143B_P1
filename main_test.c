#include <stdio.h>
#include <stdlib.h>

#include "code/io_system.h"
#include "code/file_system.h"

int main(int argc,char* argv[])
{
	
	//file_system.init(NULL);
	file_system.init(argv[1]);
	print_bitmap();
	print_blocks();
	//file_system.create("abc");
	file_system.save(argv[1]);

	return 0;
}
