#include "disk.h"


void allocate_disk(){
	add_0 = (void*) malloc(1073741824 * sizeof(char)); // allocate disk (1GB)
}

void disk_read(void* out, unsigned int block_id){
        void* Disk_Buffer = (void*)((char*)add_0 + block_id*4096);
        memcpy(out, Disk_Buffer, DB_SIZE);
}
void disk_write(void* in, unsigned int block_id){
        void* Disk_Buffer = (void*)((char*)add_0 + block_id*4096);
        memcpy(Disk_Buffer, in, DB_SIZE);
}
