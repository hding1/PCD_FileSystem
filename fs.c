// Responsible author(s): DZ and Fuheng
#include <db.h>
#include <fs.h>

void mkfs(){
	// init sb, inode, db, root
	sb_init();
	inode_list_init();
	db_init();
	pcd_mkroot();
}

db* add_0; // the first block of the disk

struct super_block{
unsigned int filesize;
unsigned int blocksize;
unsigned int MAX_NUM_INODE;
void* ilist;
void* data_block;
void* bitmap;
void* free_list;
}sb;
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
