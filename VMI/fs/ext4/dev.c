#include <stdio.h>
#include <string.h>

#include <ide.h>
#include <part.h>
#include <ext4fs.h>
#include <memalign.h>
#include "ext4_common.h"
#include "../devDriver.h"

lbaint_t part_offset;

static block_dev_desc_t *ext4fs_block_dev_desc;
static disk_partition_t *part_info;

void ext4fs_set_blk_dev(block_dev_desc_t *rbdd, disk_partition_t *info)
{
    if (rbdd->blksz != (1 << rbdd->log2blksz))
    {
        printf("block size and log to block size unmatch!\n");
        return;
    }

    ext4fs_block_dev_desc = rbdd;
    get_fs()->dev_desc = rbdd;
    part_info = info;
    part_offset = info->start;
    get_fs()->total_sect = ((uint64_t)info->size * info->blksz) >> get_fs()->dev_desc->log2blksz;
}

int ext4fs_devread(lbaint_t sector, int byte_offset, int byte_len, char *buf)
{
    unsigned block_len;
    int log2blksz = ext4fs_block_dev_desc->log2blksz;
    char sec_buf[DEFAULT_SECTOR_SIZE];
    if (ext4fs_block_dev_desc == NULL)
    {
        printf("** Invalid Block Device Descriptor (NULL)\n");
        return 0;
    }

    /* Check partition boundaries */
    if ((sector < 0) || ((sector + ((byte_offset + byte_len - 1) >> log2blksz)) >= part_info->size))
    {
        printf("%s read outside partition %llu\n", __func__, (unsigned long long)sector);
        return 0;
    }

    /* sector pass in this function is always first sector of a block, so byte_offset is offset in block.
       We need to re-calculate really sector by (byte_offset / 512), 512 is default physical sector size */
    /* We also need to re-calculate byte_offset and byte_len for new physical sector */
    /* Get the read to the beginning of a partition */
    sector += byte_offset >> log2blksz;
    byte_offset &= ext4fs_block_dev_desc->blksz - 1;

//    printf(" <%llu, %d, %d>\n", sector, byte_offset, byte_len);

    if (byte_offset != 0)
    {
        int readlen;
        /* read first part which isn't aligned with start of sector */
        if (ext4fs_block_dev_desc->block_read(ext4fs_block_dev_desc, part_info->start + sector, 1, (void *)sec_buf) != 1)
        {
            printf(" ** ext2fs_devread() read error **\n");
            return 0;
        }
        readlen = ((int)ext4fs_block_dev_desc->blksz - byte_offset) < byte_len ? ((int)ext4fs_block_dev_desc->blksz - byte_offset) : byte_len;
        memcpy(buf, sec_buf + byte_offset, readlen);
        buf += readlen;
        byte_len -= readlen;
        sector++;
    }

    if (byte_len == 0)
        return 1;

    /* read sector aligned part */
    block_len = byte_len & ~(ext4fs_block_dev_desc->blksz - 1);

    if (block_len == 0)
    {
        uint8_t p[DEFAULT_SECTOR_SIZE];

        block_len = ext4fs_block_dev_desc->blksz;
        ext4fs_block_dev_desc->block_read(ext4fs_block_dev_desc, part_info->start + sector, 1, (void *)p);
        memcpy(buf, p, byte_len);
        return 1;
    }

    if (ext4fs_block_dev_desc->block_read(ext4fs_block_dev_desc, part_info->start + sector, block_len >> log2blksz, (void *)buf) != block_len >> log2blksz)
    {
        printf(" ** %s read error - block\n", __func__);
        return 0;
    }
    block_len = byte_len & ~(ext4fs_block_dev_desc->blksz - 1);
    buf += block_len;
    byte_len -= block_len;
    sector += block_len / ext4fs_block_dev_desc->blksz;

    if (byte_len != 0)
    {
        /* read rest of data which are not in whole sector */
        if (ext4fs_block_dev_desc->block_read(ext4fs_block_dev_desc, part_info->start + sector, 1, (void *)sec_buf) != 1)
        {
            printf("* %s read error - last part\n", __func__);
            return 0;
        }
        memcpy(buf, sec_buf, byte_len);
    }
    return 1;
}

int ext4_read_superblock(char *buffer)
{
    struct ext_filesystem *fs = get_fs();
    int sect = SUPERBLOCK_START >> fs->dev_desc->log2blksz;
    int off = SUPERBLOCK_START % fs->dev_desc->blksz;

    return ext4fs_devread(sect, off, SUPERBLOCK_SIZE, buffer);
}
