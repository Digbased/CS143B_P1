#include <stdio.h>
#include <stdlib.h>
#include <criterion/criterion.h>

#include "code/io_system.h"

#define INT_LENGTH 2
#define BITMAP_SIZE sizeof(int) * (INT_LENGTH)

static char* bitmap;
extern ldisk pdisk;

void setup()
{
	bitmap = (char*)malloc(sizeof(char) * BITMAP_SIZE);	
	memset(bitmap,0,BITMAP_SIZE);
	io_system.init(NULL);
}

void teardown()
{
	free(bitmap);
	io_system.free_disk();
}

Test(bitmaps,first_test)
{
	cr_assert_eq(BITMAP_SIZE,8,"Error BITMAP_SIZE is not 8\n");
}

Test(bitmaps,second_test, .init = setup, .fini = teardown)
{
	cr_assert_eq(bitmap[0],'\0',"Error bitmap. Actual: %c | Expected: %c\n",bitmap[0],0);	
}

Test(bitmaps,third_test, .init = setup, .fini = teardown)
{
	memset(bitmap,'A',BITMAP_SIZE);
	printf("bitmap: %s\n",bitmap);
	for(int i = 0;i < BITMAP_SIZE;i += 2)
		bitmap[i] = 1;
}

Test(bitmaps, fourth_test, .init = setup, .fini = teardown)
{	
	//todo: bit manipulation
	//each bit represents a block in the data
	//0 = block is free
	//1 = block is taken
	//need to access each bit individually !

	int sample = 2;
	//goal: access all the bits in sample and print them out as binary values
	int int_size = sizeof(int) * 8;
	for(int i = 0;i < int_size;++i)
	{
		printf("%u",(sample >>  i) & 1);
	}
	printf("\n");
}

Test(bitmaps, fifth_test, .init = setup, .fini = teardown)
{
	//I want to write a function that checks to see which bit is enabled in block 0
	int i = 5;
	int isBitEnabled = (bitmap[0] >> i) & 1;
	cr_assert_eq(isBitEnabled,0,"Error fifth_test.. Actual: %d, Expected: %d\n",isBitEnabled,0);

}

//Test(init, init_no_file, .init = setup, .fini = teardown)
//{
//	int status = io_system.init(NULL);
//	cr_assert_eq(status,0,"Error init_no_file... Actual: %d, Expected: %d\n",status,0);
//}
//
//Test(init, init_file_not_exist, .init = setup, .fini = teardown)
//{
//	int status = io_system.init("saves/non_existent_file.txt");
//	cr_assert_eq(status, 1,"Error in init_file_not_exist... Actual: %d, Expected: %d\n",status,1);
//}
//
//Test(init, init_file_exist, .init = setup, .fini = teardown)
//{
//	int status = io_system.init("saves/existing_file.txt");
//	cr_assert_eq(status,2,"Error in init_file_exist... Actual: %d, Expected: %d\n",status, 2);
//}

Test(bitmaps, sixth_test, .init = setup, .fini = teardown)
{
	int bitEnabled = io_system.isBitEnabled(0);
	//this should be true since the first block of ldisk is reserved for the bitmap
	cr_assert_eq(bitEnabled,1, "Error sixth_test... Actual: %d, Expected: %d\n",bitEnabled,1);
}
