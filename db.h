// Responsible author(s): Fuheng
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "sb.h"
#ifndef DB_H_
#define DB_H_
#define DB_SIZE 4096

/*typedef struct db{
char Block[4096];
}db;*/

int db_init();
int db_allocate();
int db_free(unsigned int block_id);

//int is_db_free(unsigned int block_id);

int db_read(void* out_buffer, unsigned int block_id);
int db_write(void* in_buffer, unsigned int block_id);
#endif
