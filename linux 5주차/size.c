#include <stdio.h>

int main()
{
	char str1[10] = "123456789";
	char *str2 = "123456789";

	printf("%lu %lu\n", sizeof(str1), sizeof(str2));

	return 0;
}
