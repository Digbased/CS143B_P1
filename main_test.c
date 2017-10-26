#include <stdio.h>
#include <stdlib.h>

#include "code/io_system.h"
#include "code/file_system.h"

int main(int argc,char* argv[])
{
	
	file_system.init(NULL);
	print_bitmap();
	print_blocks();

	file_system.create("abc");
	print_bitmap();
	print_blocks();
	
	file_system.save(NULL);

	return 0;
}
