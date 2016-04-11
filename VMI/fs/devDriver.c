#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>

#include <part.h>
#include <command.h>

#include "devDriver.h"

#define CONFIG_SYS_MAX_DEVICE (1)

static int dev_curr = -1;
block_dev_desc_t dev_desc[CONFIG_SYS_MAX_DEVICE];

static uint64_t dev_bread(block_dev_desc_t *block_dev, uint64_t lba, uint64_t cnt, uint8_t* buf)
{
//printf("dev_bread lba: %d\n", lba);

    long fd;
    int64_t offset, readcnt;

    fd = open(block_dev->devPathName, O_RDONLY);
    if (fd == -1)
    {
        printf("open fail: %s\n", block_dev->devPathName);
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

    return readcnt/DEFAULT_SECTOR_SIZE;
}

static uint64_t dev_bwrite(block_dev_desc_t *block_dev, lbaint_t start,
				 lbaint_t blkcnt, const void *buffer)
{
printf("dev_bwrite: %d", start);
return 0;
//    return sata_write(block_dev->dev, start, blkcnt, buffer);
}

int dev_initialize(void)
{
    int rc;
    int i;

    for (i = 0; i < CONFIG_SYS_MAX_DEVICE; i++)
    {
        memset(&dev_desc[i], 0, sizeof(struct block_dev_desc));
        dev_desc[i].if_type = IF_TYPE_SCSI;
        dev_desc[i].dev = i;
        dev_desc[i].part_type = PART_TYPE_UNKNOWN;
        dev_desc[i].type = DEV_TYPE_HARDDISK;
        dev_desc[i].lba = 0;
        dev_desc[i].blksz = 512;
        dev_desc[i].log2blksz = LOG2(dev_desc[i].blksz);
        dev_desc[i].block_read = dev_bread;
        dev_desc[i].block_write = dev_bwrite;

// not init fs for less of devPathName which is set by user
/*        rc = init_sata(i);
        if (!rc)
        {
            rc = scan_sata(i);
            if (!rc && (dev_desc[i].lba > 0) && 
                (dev_desc[i].blksz > 0))
                init_part(&dev_desc[i]);
        }*/
    }
    dev_curr = 0;
    return rc;
}

//int sata_initialize(void) __attribute__((weak,alias("__sata_initialize")));

/*__weak*/ int sata_stop(void)
{
    int i, err = 0;

    for (i = 0; i < CONFIG_SYS_MAX_DEVICE; i++)
    {
//        err |= reset_sata(i);
    }

    if (err)
            printf("Could not reset some SATA devices\n");

    return err;
}
//int sata_stop(void) __attribute__((weak, alias("__sata_stop")));

block_dev_desc_t *get_diskdev(int dev)
{
    return (dev < CONFIG_SYS_MAX_DEVICE) ? &dev_desc[dev] : NULL;
}

block_dev_desc_t *getCurrDiskdev()
{
    return &dev_desc[dev_curr];
}

static int do_sata(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = 0;

    if (argc == 2 && strcmp(argv[1], "stop") == 0)
        return sata_stop();

    if (argc == 2 && strcmp(argv[1], "init") == 0)
    {
        if (dev_curr != -1)
            sata_stop();

        return dev_initialize();
//		return sata_initialize();
    }

    /* If the user has not yet run `sata init`, do it now */
    if (dev_curr == -1)
        if (dev_initialize())
            return 1;

    switch (argc)
    {
        case 0:
        case 1:
            return CMD_RET_USAGE;
        case 2:
            if (strncmp(argv[1],"inf", 3) == 0)
            {
                int i;
                puts('\n');
                for (i = 0; i < CONFIG_SYS_MAX_DEVICE; ++i)
                {
                    if (dev_desc[i].type == DEV_TYPE_UNKNOWN)
                        continue;
                    printf ("SATA device %d: ", i);
                    dev_print(&dev_desc[i]);
                }
                return 0;
            }
            else if (strncmp(argv[1],"dev", 3) == 0)
            {
                if ((dev_curr < 0) || (dev_curr >= CONFIG_SYS_MAX_DEVICE))
                {
                    puts("\nno SATA devices available\n");
                    return 1;
                }
                printf("\nSATA device %d: ", dev_curr);
                dev_print(&dev_desc[dev_curr]);
                return 0;
            }
            else if (strncmp(argv[1],"part",4) == 0)
            {
                int dev, ok;

                for (ok = 0, dev = 0; dev < CONFIG_SYS_MAX_DEVICE; ++dev)
                {
                    if (dev_desc[dev].part_type != PART_TYPE_UNKNOWN)
                    {
                            ++ok;
                            if (dev)
                                puts ('\n');
                            print_part(&dev_desc[dev]);
                    }
                }
                if (!ok)
                {
                    puts("\nno SATA devices available\n");
                    rc ++;
                }
                return rc;
            }
            return CMD_RET_USAGE;
        case 3:
            if (strncmp(argv[1], "dev", 3) == 0)
            {
                int dev = (int)strtoul(argv[2], NULL, 10);

                printf("\nSATA device %d: ", dev);
                if (dev >= CONFIG_SYS_MAX_DEVICE)
                {
                    puts ("unknown device\n");
                    return 1;
                }
                dev_print(&dev_desc[dev]);

                if (dev_desc[dev].type == DEV_TYPE_UNKNOWN)
                    return 1;

                dev_curr = dev;

                puts("... is now current device\n");

                return 0;
            }
            else if (strncmp(argv[1], "part", 4) == 0)
            {
                int dev = (int)strtoul(argv[2], NULL, 10);

                if (dev_desc[dev].part_type != PART_TYPE_UNKNOWN)
                {
                    print_part(&dev_desc[dev]);
                }
                else
                {
                    printf("\nSATA device %d not available\n", dev);
                    rc = 1;
                }
                return rc;
            }
            return CMD_RET_USAGE;

        default: /* at least 4 args */
            if (strcmp(argv[1], "read") == 0)
            {
                    ulong addr = strtoull(argv[2], NULL, 16);
                    ulong cnt = strtoull(argv[4], NULL, 16);
                    ulong n;
                    lbaint_t blk = strtoull(argv[3], NULL, 16);

                    printf("\nSATA read: device %d block # %ld, count %ld ... ", dev_curr, blk, cnt);

//                    n = sata_read(dev_curr, blk, cnt, (uint32_t *)addr);

                    /* flush cache after read */
//                    flush_cache(addr, cnt * dev_desc[dev_curr].blksz);

                    printf("%ld blocks read: %s\n",
                            n, (n==cnt) ? "OK" : "ERROR");
                    return (n == cnt) ? 0 : 1;
            }
            else if (strcmp(argv[1], "write") == 0)
            {
                ulong addr = strtoull(argv[2], NULL, 16);
                ulong cnt = strtoull(argv[4], NULL, 16);
                ulong n;

                lbaint_t blk = strtoull(argv[3], NULL, 16);

                printf("\nSATA write: device %d block # %ld, count %ld ... ", dev_curr, blk, cnt);

                n = dev_bwrite(dev_curr, blk, cnt, (uint32_t *)addr);

                printf("%ld blocks written: %s\n",
                        n, (n == cnt) ? "OK" : "ERROR");
                return (n == cnt) ? 0 : 1;
            }
            else
            {
                return CMD_RET_USAGE;
            }

            return rc;
    }
}
#if 0
U_BOOT_CMD(
	sata, 5, 1, do_sata,
	"SATA sub system",
	"init - init SATA sub system\n"
	"sata stop - disable SATA sub system\n"
	"sata info - show available SATA devices\n"
	"sata device [dev] - show or set current device\n"
	"sata part [dev] - print partition table\n"
	"sata read addr blk# cnt\n"
	"sata write addr blk# cnt"
);
#endif
