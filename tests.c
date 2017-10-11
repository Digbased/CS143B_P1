#include "code/io_system.h"
#include "code/file_system.h"

#include <criterion/criterion.h>

Test(simple, test)
{
	cr_expect(1 == 1,"hello there!");
}
