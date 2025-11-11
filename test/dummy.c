#include <test/base.h>

TEST(sanity_addition)
{
	return 3 + 4 == 7 ? 0 : 1;
}
