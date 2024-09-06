#include <stdio.h>
#include <unistd.h>

int main()
{

	int a;
	//close(0);                      // scanf 작동안함
	scanf("%d", &a);               // fd : 0(stdin)
	//close(1);  // 출력안됨
	printf("Hello World\n");	   // fd : 1(stdout)
	//close(2); // 에러 안뜸
	perror("Hello Error");         // fd : 2(stderr)
	
	return 0;
}

