#ifndef PCD_FILESYSTEM_FS_H_
#define PCD_FILESYSTEM_FS_H_

// Responsible author(s): DZ

#include "inode.h"
#include "sb.h"
#include "db.h"
#include <stdlib.h>

int mkfs();
void initialize(const char* path);
void freefs();

#endif //PCD_FILESYSTEM_FS_H_
