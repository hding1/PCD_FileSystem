//author: Fuheng
#include <string.h>
#include <stdlib.h>

#ifndef DISK_H_
#define DISK_H_

#define DB_SIZE 4096

void* add_0; 
// Address of the first block of the diskextern void* add_0

void allocate_disk();
void disk_read(void* out, unsigned int block_id);
void disk_write(void* in, unsigned int block_id);

#endif
