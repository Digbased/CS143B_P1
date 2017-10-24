#include <stdio.h>
#include <stdlib.h>
#include <criterion/criterion.h>

#include "code/ldisk.h"
#include "code/file_system.h"
#include "code/io_system.h"

#define INT_LENGTH 2
#define BITMAP_SIZE sizeof(int) * (INT_LENGTH)

static char* bitmap;
extern ldisk pdisk;

void setup()
{
	bitmap = (char*)malloc(sizeof(char) * BITMAP_SIZE);	
	memset(bitmap,0,BITMAP_SIZE);
	file_system.init(NULL);
}

void teardown()
{
	free(bitmap);
	file_system.free_disk();
}

Test(bitmaps,first_test)
{
	cr_assert_eq(BITMAP_SIZE,8,"Error BITMAP_SIZE is not 8\n");
}

Test(bitmaps,second_test, .init = setup, .fini = teardown)
{
	cr_assert_eq(bitmap[0],'\0',"Error bitmap. Actual: %c | Expected: %c\n",bitmap[0],0);	
}

Test(bitmaps, fifth_test, .init = setup, .fini = teardown)
{
	//I want to write a function that checks to see which bit is enabled in block 0
	int i = 5;
	int isBitEnabled = (bitmap[0] >> i) & 1;
	cr_assert_eq(isBitEnabled,0,"Error fifth_test.. Actual: %d, Expected: %d\n",isBitEnabled,0);

}

Test(bitmaps, sixth_test, .init = setup, .fini = teardown)
{
	int bitEnabled = file_system.isBitEnabled(0);
	//this should be true since the first block of ldisk is reserved for the bitmap
	cr_assert_eq(bitEnabled,1, "Error sixth_test... Actual: %d, Expected: %d\n",bitEnabled,1);
}

Test(bitmaps, seventh_test, .init = setup, .fini = teardown)
{
	int bitEnabled = file_system.isBitEnabled(32);
	cr_assert_eq(bitEnabled,0, "Error seventh_test... Actual: %d, Expected: %d\n",bitEnabled,0);
	
}

Test(bitmaps, enableBit_simple, .init = setup, .fini = teardown)
{
	file_system.enableBit(5);	
	int bitEnabled = file_system.isBitEnabled(5);
	cr_assert_eq(bitEnabled,1, "Error enableBit_simple... Actual: %d, Expected: %d\n",bitEnabled,1);
}

Test(bitmaps, enableBit_thorough, .init = setup, .fini = teardown)
{
	file_system.enableBit(56);
	int isEnabled = file_system.isBitEnabled(56);
	cr_assert_eq(isEnabled,1, "Error enableBit_thorough at pdisk.buf bit at %d... Actual: %d, Expected: %d\n",56,isEnabled, 1);
	for(int i = 8;i < 64;++i)
	{
		if(i == 56) continue;
		int bitEnabled = file_system.isBitEnabled(i);
		cr_assert_eq(bitEnabled,0, "Error enableBit_thorough at pdisk.buf bit %d... Actual: %d, Expected: %d\n",i,bitEnabled,0);
	}	
}

Test(bitmaps, enableBit_evens, .init = setup, .fini = teardown)
{
	for(int i = 8;i < 64;i += 2)
		file_system.enableBit(i);

	for(int i = 8;i < 64;i += 2)
	{
		int bitEnabled = file_system.isBitEnabled(i);
		cr_assert_eq(bitEnabled,1,"Error enableBit_evens at pdisk.buf bit %d... Actual: %d, Expected: %d\n",i,bitEnabled,1);
	}

	for(int i = 9;i < 64;i += 2)
	{
		int bitEnabled = file_system.isBitEnabled(i);
		cr_assert_eq(bitEnabled,0,"Error enableBit_evens at pdisk.buf bit %d... Actual: %d, Expected: %d\n",i,bitEnabled,0);
	}
}


Test(bitmaps, disableBit_simple, .init = setup, .fini = teardown)
{
	file_system.enableBit(5);
	file_system.disableBit(5);	
	int bitEnabled = file_system.isBitEnabled(5);
	cr_assert_eq(bitEnabled,0, "Error disableBit_simple... Actual: %d, Expected: %d\n",bitEnabled,0);
}

Test(bitmaps, disableBit_thorough, .init = setup, .fini = teardown)
{
	file_system.enableBit(56);
	int isE = file_system.isBitEnabled(56);
	cr_assert_eq(isE,1,"Error disableBit_thorough\n");

	file_system.disableBit(56);
	int isEnabled = file_system.isBitEnabled(56);
	cr_assert_eq(isEnabled,0, "Error disableBit_thorough at pdisk.buf bit at %d... Actual: %d, Expected: %d\n",56,isEnabled, 0);
	for(int i = 7;i < 64;++i)
	{
		if(i == 56) continue;
		int bitEnabled = file_system.isBitEnabled(i);
		cr_assert_eq(bitEnabled,0, "Error enableBit_thorough at pdisk.buf bit %d... Actual: %d, Expected: %d\n",i,bitEnabled,0);
	}	
}

Test(bitmaps, disableBit_evens, .init = setup, .fini = teardown)
{
	for(int i = 0;i < 64;i += 2)
		file_system.enableBit(i);

	for(int i = 0;i < 64;i += 2)
		file_system.disableBit(i);

	for(int i = 0;i < 64;i += 2)
	{
		int bitEnabled = file_system.isBitEnabled(i);
		cr_assert_eq(bitEnabled,0,"Error disableBit_evens at pdisk.buf bit %d... Actual: %d, Expected: %d\n",i,bitEnabled,0);
	}

}

