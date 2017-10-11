#include <criterion/criterion.h>

Test(simple, test)
{
	cr_expect(1 == 1,"hello world!");
}

Test(simple, another_test)
{
	cr_assert(0,"hello there!");
}
