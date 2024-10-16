#include <stdio.h>
#include <stdlib.h>

void memory_leak(){
    int ndx;
    char *p_leak;
    p_leak = malloc(10);
    //for(ndx = 0; ndx <= 10; ndx++);
    for(ndx = 0; ndx < 10; ndx++);
        p_leak[ndx] = 'a';
    for(ndx = 0; ndx < 10; ndx++)
        printf("%c", p_leak[ndx]);
    //free(p_leak);
}

int main()
{
    memory_leak();
}
