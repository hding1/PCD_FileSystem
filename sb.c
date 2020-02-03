#include <sb.h>

void sb_init(){
	sb.filesize = 100;
	sb.blocksize = 100;
	sb.MAX_NUM_INODE = 100;
       	sb.ilist = &(add_0 + 3);
	sb.datablock=&(add_0+100);
	sb.bitmap=&(add_0+1)
	sb.free_list=&(add_0_+103);	
	
	char* sb_bytes = static_cast<char*>(static_cast<void*>(&sb));
	db *input = malloc(sizeof(db));
	db->block = &sb_bytes;

	disk_write(input, add_0);
}

int sb_read(){

}

int sb_write(){

}