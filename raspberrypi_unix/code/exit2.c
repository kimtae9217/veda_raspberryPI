#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	printf("Hello World");
	fflush(stdout);
	_exit(0);
}
