#ifndef PCD_FILESYSTEM_DB_H_
#define PCD_FILESYSTEM_DB_H_

// Responsible author(s): FZ

#include <stdlib.h>
#include "sb.h"
#ifndef DB_H_
#define DB_H_
#define DB_SIZE 4096

/*typedef struct db{
char Block[4096];
}db;*/

void db_init();
unsigned int db_allocate();
int db_free(unsigned int block_id);

<<<<<<< HEAD
#endif //PCD_FILESYSTEM_DB_H_
=======

int db_read(void* out_buffer, unsigned int block_id);
int db_write(void* in_buffer, unsigned int block_id);
void disk_read(void* out_buffer, unsigned int block_id);
void disk_write(void* in_buffer, unsigned int block_id);
#endif
>>>>>>> layer1
