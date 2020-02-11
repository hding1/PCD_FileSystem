#ifndef PCD_FILESYSTEM_FS_H_
#define PCD_FILESYSTEM_FS_H_

// Responsible author(s): DZ

<<<<<<< HEAD
#include <stdlib.h>

#include "inode.h"
#include "sb.h"

extern void* add_0; // Address of the first block of the disk

=======
#include "inode.h"
#include "sb.h"
#include "db.h"
#include <stdlib.h>

>>>>>>> layer1
void mkfs();

#endif //PCD_FILESYSTEM_FS_H_