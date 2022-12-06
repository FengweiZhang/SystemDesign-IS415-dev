#include <stdio.h>
#include <stdlib.h>


int main(){
    char buf[10000];
    char filename1[100] = "./testfile/test1";
    char filename2[100] = "./testfile/test2";
    char filename3[100] = "./testfile/test3";
    char filename4[100] = "./testfile/test4";
    char filename5[100] = "./testfile/test2.l";
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
        else
        {
            printf("read test1 succeed\n");
        }
        fclose(pf);
        
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
        else
        {
            printf("read test2 succeed\n");
        }
        fclose(pf);
    }

    pf = fopen(filename5, "r");
    if (pf == NULL)
    {
        perror("open test2.l failed");
    }
    else
    {
        
        if (fscanf(pf, "%s", buf) < 0)
        {
            perror("read test2.l failed");
        }
        else
        {
            printf("read test2.l succeed\n");
        }
        fclose(pf);
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
        else
        {
            printf("read test2 succeed\n");
        }
        fclose(pf);
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
        else
        {
            printf("read test2 succeed\n");
        }
        fclose(pf);
    }
    return 0;
    
}