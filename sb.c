#include <sb.h>

void sb_init(){
	sb sb;
	sb.filesize = 1073741824;
	sb.blocksize = 4096;
	sb.MAX_NUM_INODE = 4096;
       	sb.ilist = &(add_0 + 3*4096);
	sb.datablock=&(add_0 + 131*4096);
	sb.bitmap=&(add_0+1*4096)
	sb.free_list=&(add_0_+103);	
	
	char* sb_bytes = static_cast<char*>(static_cast<void*>(&sb));
	
	db *input = malloc(sizeof(db));
	
	input->block = &sb_bytes;

	disk_write(input, add_0);
}

int sb_read(){

}

int sb_write(){

}
