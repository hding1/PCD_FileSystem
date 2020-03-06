// Responsible author(s): DZ and Fuheng
#include "fs.h"
#include "syscall.h"

int mkfs(){
	// init sb, inode, db, root

	if(allocate_disk() == -1){
		printf("disk allocation failed! \n");
		return -1;
	}
	sb_init();
	
	if(inode_bitmap_init() == -1){
		printf("inode_bitmap_init() FAILED!\n");
		return -1;
	}
	
	if(inode_list_init() == -1){
		printf("inode_list_init() FAILED!\n");
		return -1;
	}

	db_init();

	allocate_cache();
	list_init();
	hash_init();

	pcd_mkroot();

	return 0;

}
void freefs(){

	free_disk();
	deallocate_cache();
	list_free();
	hash_free();

}

