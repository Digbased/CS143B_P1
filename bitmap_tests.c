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

	pdisk.logical_block_size = L;
	pdisk.block_length = B;
		

}

void teardown()
{
	free(bitmap);
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
