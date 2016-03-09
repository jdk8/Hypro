//#include <fs.h>

// system
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// VMI
#include "devDriver.h"
#include "fs.h"

#define COMMAND_MAX_NUM  (8)
#define COMMAND_MAX_SIZE (256)

void help()
{
    printf("copy <source> <destination>           : Copy file"\
           "exit                                  : Exit fs-op shell\n"\
           "help                                  : List all commands\n"\
           "ls <PathName>                         : List directory\n"\
           "setfs <DevPathName> <PartitionNumber> : Set file system in device and partition\n");
}

void quit()
{
    // free memory and resource
}

void setfs(char *devStr, char *partNumStr)
{
    block_dev_desc_t *devDesc;
    int partNum, part;

    devDesc = getCurrDiskdev();
    strcpy(devDesc->devPathName, devStr);

    // make sure MBR is dos type
    init_part(devDesc);

    // print all partitions
    print_part(devDesc);

    partNum = strtoul(partNumStr, 0, 0);
    disk_partition_t *diskPart = getDiskPartition();
    part = get_partition_info(devDesc, partNum, diskPart);
    if (get_partition_info(devDesc, partNum, diskPart) < 0)
    {
        return;
    }

    fs_setFsType(devDesc, diskPart);
}

int main(int argc, char* argv[])
{
    char cmdBuf[COMMAND_MAX_NUM][COMMAND_MAX_SIZE];

    // initialize dev read/write func
    dev_initialize();

    // give input prompt
    printf(">");

    while (scanf("%s", cmdBuf[0]))
    {
        if (strcmp(cmdBuf[0], "quit") == 0 )
        {
            quit();
            return 0;
        }
        else if (strcmp(cmdBuf[0], "help") == 0)
        {
            help();
        }
        else if (strcmp(cmdBuf[0], "setfs") == 0)
        {
            scanf("%s %s", cmdBuf[1], cmdBuf[2]);
            setfs(cmdBuf[1], cmdBuf[2]);
        }
        else if (strcmp(cmdBuf[0], "ls") == 0)
        {
            scanf("%s", cmdBuf[1]);
            fs_ls(cmdBuf[1]);
        }
        else if (strcmp(cmdBuf[0], "copy") == 0)
        {
            scanf("%s %s", cmdBuf[1], cmdBuf[2]);
            fs_copy(cmdBuf[1], cmdBuf[2]);
        }
        else
        {
            printf("Unknown command! Please try help\n");
        }

        printf(">");
    }
}
