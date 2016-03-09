#ifndef _HYPERCALL_H
#define _HYPERCALL_H

#include <stdint.h>

#define DEFAULT_SECTOR_SIZE (512)
#define DEFAULT_BLOCK_SIZE  (4096)

/* 
   dev: device, eg: "/dev/sda1"
   lba: Logical Block Address
   cnt: sector count
   buf: buffer to get sectors back
   return: acctually sectore read
*/
uint64_t HyperCall_FS_Read(int8_t* dev, uint64_t lba, uint64_t cnt, int8_t* buf);

#endif
