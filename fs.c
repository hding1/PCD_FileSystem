// Responsible author(s): DZ and Fuheng
#include "fs.h"
#include "syscall.h"

void mkfs(){
	// init sb, inode, db, root

	allocate_disk();
	sb_init();
	
	if(inode_bitmap_init() == -1){
		printf("inode_bitmap_init() FAILED!\n");
		return;
	}
	
	if(inode_list_init() == -1){
		printf("inode_list_init() FAILED!\n");
		return;
	}

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

