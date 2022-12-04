#include <stdio.h>
#include <stdlib.h>


int main(){
    char buf[10000];
    char filename[100] = "/home/ubuntu/Workspace/123";
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        // perror("open file for reading");
        exit(0);
    }
    fscanf(fp, "%s", buf);
    perror("read");
    fclose(fp);
}