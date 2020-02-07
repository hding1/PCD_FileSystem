#ifndef PCD_FILESYSTEM_FS_H_
#define PCD_FILESYSTEM_FS_H_

// Responsible author(s): DZ

#include <inode.h>
#include <sb.h>
#include <stdlib.h>

extern void* add_0; // Address of the first block of the disk

void mkfs();

#endif //PCD_FILESYSTEM_FS_H_