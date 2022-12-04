#include <stdio.h>
#include <stdlib.h>


int main(){
    char buf[10] = "ttt\n";
    char filename[100] = "/home/ubuntu/Workspace/123";
    FILE *pf = fopen(filename, "a");
    if (pf == NULL)
    {
        perror("open file for reading");
        exit(0);
    }
    fputs(buf, pf);
    perror("write failed");
    fclose(pf);
}