#include <stdio.h>
#include <stdlib.h>


int main(){
    char buf[10000];
    char filename1[100] = "/home/ubuntu/test/test1";
    char filename2[100] = "/home/ubuntu/test/test2";
    char filename3[100] = "/home/ubuntu/test/test3";
    char filename4[100] = "/home/ubuntu/test/test4";
    FILE *pf = fopen(filename1, "r");
    if (pf == NULL)
    {
        perror("open test1 failed");
    }
    else
    {
        
        if (fscanf(pf, "%s", buf) < 0)
        {
            perror("read test1 failed");
        }
        fclose(pf);
        printf("read test1 succeed\n");
    }

    pf = fopen(filename2, "r");
    if (pf == NULL)
    {
        perror("open test2 failed");
    }
    else
    {
        
        if (fscanf(pf, "%s", buf) < 0)
        {
            perror("read test2 failed");
        }
        fclose(pf);
        printf("read test1 succeed\n");
    }
    
    pf = fopen(filename3, "r");
    if (pf == NULL)
    {
        perror("open test3 failed");
    }
    else
    {
        
        if (fscanf(pf, "%s", buf) < 0)
        {
            perror("read test3 failed");
        }
        fclose(pf);
        printf("read test3 succeed\n");
    }

    pf = fopen(filename4, "r");
    if (pf == NULL)
    {
        perror("open test4 failed");
    }
    else
    {
        
        if (fscanf(pf, "%s", buf) < 0)
        {
            perror("read test4 failed");
        }
        fclose(pf);
        printf("read test4 succeed\n");
    }
    return 0;
    
}