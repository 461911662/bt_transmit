#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char checkSum(unsigned char *pcBuf, unsigned char lenght)
{
    unsigned char i;
    char ucCheckSum=0;
    if(pcBuf == NULL || lenght < 0)
    {
        return 255;
    }

    for(i=0; i<lenght; i++)
    {
        ucCheckSum += pcBuf[i]; 
    }

    return ucCheckSum;
}

void usage()
{
    printf("\nUsage:");
    printf("\n checkSum:\n");
    printf(" ./checkSum [data...] datalen");
    printf(" eg: ./checkSum 0x11 0x22 0x33 0x3\n\n");
    _exit(0);
}

int main(char argc, char *argv[])
{
    if(argc < 3 || strtol(argv[argc-1], NULL, 16) < 2)
    {
        usage();
    }
    unsigned char aucData[255] = {0};
    char acData[255] = {0};
    int i, iLen;
    char result = 0;
    iLen = strtol(argv[argc-1], NULL, 16);
    if(argc != iLen + 2)
    {
        usage();
    }

    for(i=0; i<=iLen; i++)
    {
        aucData[i] = strtol(argv[i], NULL, 16);
        strcat(acData, argv[i]);
        strcat(acData, " ");
    }
    
    result = (-checkSum(aucData, iLen+1))&0xFF;
    printf(" %sCheckSum Result is %#x\n", acData, result);
    return 0;
}
