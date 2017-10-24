#include <stdio.h>
#include <string.h>
#include <criterion/criterion.h>

#include "code/ldisk.h"
#include "code/io_system.h"
#include "code/file_system.h"

#define BLOCK_LEN 64
#define LOGICAL_LEN 64

//global variables!
extern ldisk pdisk;
static char* buffer;

void setup()
{
	//puts("Runs before the test");
	//allocate main memory	
	buffer = (char*)malloc(sizeof(char) * BLOCK_LEN);

	//allocate ldisk from main memory (ram)
	file_system.init(NULL);	
}

void teardown()
{
	//puts("Runs after the test");
	free(buffer);
	file_system.free_disk();
}

Test(simple, the_test, .init = setup, .fini = teardown)
{
	cr_assert(1,"failure this is zero!");
}

Test(io_interface,read_block_simple, .init = setup, .fini = teardown)
{

	//read block copies the logical block ldisk[i] into main memory starting at the location
	//specified by the pointer
	
	int testIndex = 1;
	io_system.read_block(testIndex,buffer,sizeof(buffer));

//	printf("pdisk block length: %d\n",pdisk.block_length);

	for(int i = 0;i < sizeof(buffer);++i)
		cr_assert(pdisk.buf[testIndex][i] == buffer[i],"test failed in read_block_simple at index[%d], actual: %c, expected: %c",i, buffer[i],pdisk.buf[testIndex][i]);

}

Test(io_interface,write_block_simple, .init = setup, .fini = teardown)
{
	//write block copies the number of character corresponding to the block length, B, from main memory
	//starting at the location specified by the pointer p, into the logical block ldisk[i].
//	puts("write_block_simple");

	for(int i = 0;i < BLOCK_LEN; ++i)
		buffer[i] = 'a';


	int testIndex = 1;	
	int status = io_system.write_block(testIndex,buffer,sizeof(buffer));
	cr_assert_eq(status,1,"test failed in write_block_simple. actual: %d, expected: %d",status,1);
	
	for(int i = 0;i < sizeof(buffer); ++i)
		cr_assert_eq(pdisk.buf[testIndex][i],buffer[i],"test failed in write_block_simple at index[%d], actual: %c, expected: %c\n",i ,pdisk.buf[testIndex][i], buffer[i]);
}

//Test(file_interface,simple_create, .init = setup, .fini = teardown)
//{
//	file_system.create("Testfile.txt");
//}
//
//Test(file_interface, simple_destroy, .init = setup, .fini = teardown)
//{
//	file_system.destroy("Testfile.txt");
//}
//
//Test(file_interface, simple_open)
//{
//	file_system.open("Testfile.txt");
//}
//
//Test(file_interface, simple_close)
//{
//	file_system.close(1);
//}
//
//Test(file_interface, simple_read)
//{
//	file_system.read(1,NULL,5);
//}
//
//Test(file_interface, simple_write)
//{
//	file_system.write(1,NULL,5);
//}
//
//Test(file_interface, simple_lseek)
//{
//	file_system.lseek(1,5);
//}
//
//Test(file_interface, simple_directory)
//{
//	file_system.directory();
//}


Test(io_interface, read_block_integer, .init = setup, .fini = teardown)
{
	int number;
	int logical_index = 0;
	io_system.read_block(logical_index,(char*)&number,sizeof(number));
	cr_assert_eq(number,127,"read_block_integer~ actual: %d; expected: %d\n",number,127);
	
	logical_index = 1;
	for(int i = 0;i < BLOCK_LEN;++i)
	{
		pdisk.buf[logical_index][i] = 'a';
	}
	
	io_system.read_block(logical_index,(char*)&number,sizeof(number));
	// "aaaa" as an integer value = 1633771873
	int eValue = *(int*)pdisk.buf[logical_index];
	cr_assert_eq(number,eValue,"read_block_integer actual: %d, expected: %d\n",number, eValue);


}

Test(io_interface, write_block_integer, .init = setup, .fini = teardown)
{
	int number = 25;
	int logical_index = 5;
	io_system.write_block(logical_index,(char*)&number,sizeof(number));
	
	int eValue = *(int*)pdisk.buf[logical_index];
	cr_assert_eq(number, eValue, "write_block_integer actual: %d, expected: %d\n",number,eValue);

	char* cNumber = (char*)&number;
	for(int i = 0;i < sizeof(number);++i)
	{
		cr_assert(pdisk.buf[logical_index][i] == cNumber[i],"write_block_integer actual: %c, expected: %c\n",pdisk.buf[logical_index][i],cNumber[i]);
	}
}

