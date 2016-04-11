#ifndef __EXT4_COMMON__
#define __EXT4_COMMON__

#include <string.h>

#include <ext_common.h>
#include <ext4fs.h>
#include <malloc.h>
#include <memalign.h>


#define YES		1
#define NO		0
#define RECOVER	1
#define SCAN		0

#define S_IFLNK		0120000		/* symbolic link */
#define BLOCK_NO_ONE		1

#define SUPERBLOCK_START	(2 * 512)
#define SUPERBLOCK_SIZE	1024

#define F_FILE			1

int ext4fs_read_inode(struct ext2_data *data, int ino, struct ext2_inode *inode);
int ext4fs_read_file(struct ext2fs_node *node, loff_t pos, loff_t len, char *buf, loff_t *actread);
int ext4fs_find_file(const char *path, struct ext2fs_node *rootnode, struct ext2fs_node **foundnode, int expecttype);
int ext4fs_iterate_dir(struct ext2fs_node *dir, char *name, struct ext2fs_node **fnode, int *ftype);

uint32_t ext4fs_div_roundup(uint32_t size, uint32_t n);
int ext4fs_checksum_update(unsigned int i);
int ext4fs_get_parent_inode_num(const char *dirname, char *dname, int flags);
void ext4fs_update_parent_dentry(char *filename, int *p_ino, int file_type);
long int ext4fs_get_new_blk_no(void);
int ext4fs_get_new_inode_no(void);
void ext4fs_reset_block_bmap(long int blockno, unsigned char *buffer, int index);
int ext4fs_set_block_bmap(long int blockno, unsigned char *buffer, int index);
int ext4fs_set_inode_bmap(int inode_no, unsigned char *buffer, int index);
void ext4fs_reset_inode_bmap(int inode_no, unsigned char *buffer, int index);
int ext4fs_iget(int inode_no, struct ext2_inode *inode);
void ext4fs_allocate_blocks(struct ext2_inode *file_inode, unsigned int total_remaining_blocks, unsigned int *total_no_of_block);
void put_ext4(uint64_t off, void *buf, uint32_t size);

#endif
