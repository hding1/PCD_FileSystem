// Responsible author(s): DZ and Fuheng
#include "fs.h"
#include "pcd_fuse.h"

void mkfs(){
	// init sb, inode, db, root

	allocate_disk();
	sb_init();
	inode_bitmap_init();
	inode_list_init();
	db_init();
	pcd_mkroot();
}
void freefs(){

	free_disk();
}

