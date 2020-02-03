// Responsible author(s): DZ and Fuheng
#include <db.h>
#include <fs.h>

void mkfs(){
	// init sb, inode, db, root

	add_0 = (void*) malloc(1073741824 * sizeof(char)); // allocate disk (1GB)

	sb_init();
	inode_list_init();
	db_init();
	pcd_mkroot();
	
}

