#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

int main(int argc, char **arv){

	int i, j; 
	time_t rawtime;
	struct tm *tm;
	char buf[BUFSIZ];
	struct timeval mytiem;

	time(&rawtime);
	printf("time : %u\n", (unsigned)rawtime);

	return 0;


