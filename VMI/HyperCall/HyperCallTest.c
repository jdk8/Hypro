#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "HyperCall.h"

#define COLOR_NONE                 "\033[0m"
#define FONT_COLOR_RED             "\033[0;31m"
#define FONT_COLOR_BLUE            "\033[1;34m"
#define BACKGROUND_COLOR_RED       "\033[41m"
#define BACKGROUND_YELLOW_FONT_RED "\033[43;31m"

int main(int argc, char* argv[])
{
    int i, j, column = 32, line = 16;

    if (argc != 5)
    {
        goto tip;
    }

    if (strcmp(argv[1], "fs") == 0)
    {
        if (strcmp(argv[2], "read") == 0)
        {
            uint64_t blocknumber = strtoull(argv[4], 0, 10);
            printf("Read Block: %llu (%#010llX)\n", (unsigned long long)blocknumber, (unsigned long long)blocknumber);

            // read disk block
            char buf[DEFAULT_BLOCK_SIZE];
            HyperCall_FS_Read(argv[3], blocknumber, 1, buf);

            // print disk block
            for (i = 0 ; i < line; i++)
            {
                for (j = 0; j < column; j++)
                {
                    (buf[i * column + j] != 0) ? printf(BACKGROUND_YELLOW_FONT_RED"%02X"COLOR_NONE" ", (uint8_t)(buf[i * column + j])) : printf("%02X ", buf[i * column + j]);
                }
                printf("\n");
            }
	    return 0;
        }
    }
    else
        goto tip;

tip:
    printf("HyperCallTest fs read device sectorNumber\n"\
           "eg: HyperCallTest fs read /dev/sda 0\n");
    return -1;
}

