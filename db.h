// Responsible author(s): FZ

#include <stdlib.h>

typedef struct db{
char Block[4096];
}db;

void db_init();
unsigned int db_allocate();
int db_free(unsigned int block_id);
int db_read(db* out_buffer, unsigned int block_id);
int db_write(db* in_buffer, unsigned int block_id);
void disk_read(db* out_buffer, unsigned int block_id);
void disk_write(db* in_buffer, unsigned int block_id);

