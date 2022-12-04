#include <stdio.h>
#include <stdlib.h>


int main(){
    char buf[10] = "ttt\n";
    char filename1[100] = "/home/ubuntu/test/test1";
    char filename2[100] = "/home/ubuntu/test/test2";
    char filename3[100] = "/home/ubuntu/test/test3";
    char filename4[100] = "/home/ubuntu/test/test4";
    FILE *pf = fopen(filename1, "a");
    if (pf == NULL)
    {
        perror("open test1 failed");
    }
    else
    {
        
        if (fputs(buf, pf) < 0)
        {
            perror("write test1 failed");
        }
        fclose(pf);
        printf("write test1 succeed\n");
    }

    pf = fopen(filename2, "a");
    if (pf == NULL)
    {
        perror("open test2 failed");
    }
    else
    {
        
        if (fputs(buf, pf) < 0)
        {
            perror("write test2 failed");
        }
        fclose(pf);
        printf("write test1 succeed\n");
    }
    
    pf = fopen(filename3, "a");
    if (pf == NULL)
    {
        perror("open test3 failed");
    }
    else
    {
        
        if (fputs(buf, pf) < 0)
        {
            perror("write test3 failed");
        }
        fclose(pf);
        printf("write test3 succeed\n");
    }

    pf = fopen(filename4, "a");
    if (pf == NULL)
    {
        perror("open test4 failed");
    }
    else
    {
        
        if (fputs(buf, pf) < 0)
        {
            perror("write test4 failed");
        }
        fclose(pf);
        printf("write test4 succeed\n");
    }
    return 0;
    
}