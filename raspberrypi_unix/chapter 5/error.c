#include <fcntl.h>
#include <unistd.h>

int main()
{
	write(1, "Hello World\n", 12); // stdin
//	write(2, "Hello World\n", 12); // stderr
	return 0;
}

