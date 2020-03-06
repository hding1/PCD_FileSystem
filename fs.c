// Responsible author(s): DZ and Fuheng
#include "fs.h"
#include "syscall.h"

int mkfs(){
	// init sb, inode, db, root

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

	pcd_mkroot();

	return 0;

}
void initialize(const char* path){
	allocate_disk(path);
	allocate_cache();
	list_init();
	hash_init();
}
void freefs(){
	free_disk();
	deallocate_cache();
	list_free();
	hash_free();

}

