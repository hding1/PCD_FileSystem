// Responsible author(s): DZ and Fuheng
#include "fs.h"
#include "syscall.h"

void mkfs(){
	// init sb, inode, db, root

	allocate_disk();
	sb_init();
	inode_bitmap_init();
	inode_list_init();
	db_init();

	allocate_cache();
	list_init();
	hash_init();

	pcd_mkroot();

}
void freefs(){

	free_disk();
	deallocate_cache();
	list_free();
	hash_free();

}

