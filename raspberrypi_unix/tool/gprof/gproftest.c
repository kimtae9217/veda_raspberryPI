#include <stdio.h>
#include <math.h>

void function_call(int n)
{
    printf("Value : %f\n", sin(n));
}

int main()
{
    for(int i = 0; i < 10000; i++)
        function_call(i);

    return 0;
}
