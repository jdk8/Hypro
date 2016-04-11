#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

#include "HyperCall.h"

uint64_t HyperCall_FS_Read(int8_t* dev, uint64_t lba, uint64_t cnt, int8_t* buf)
{
    long fd;
    int64_t offset, readcnt;

    fd = open(dev, O_RDONLY | O_SYNC);
    if (fd == -1)
    {
        printf("open fail: %s\n", dev);
        return 0;
    }

    offset = lseek(fd, lba * DEFAULT_SECTOR_SIZE, SEEK_SET);
    if (offset != lba * DEFAULT_SECTOR_SIZE)
    {
        printf("lseek fail: %llu sector\n", (unsigned long long)lba);
        close(fd);
        return 0;
    }

    readcnt = read(fd, buf, cnt*DEFAULT_SECTOR_SIZE);

    fsync(fd);

    close(fd);

    return readcnt / DEFAULT_SECTOR_SIZE;
}
