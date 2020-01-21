#include <fs.h>
#include <inode.h>

void mkfs(){
	// init sb, inode, db, root
	sb_init();
	inode_list_init();
	db_init();
	pcd_mkroot();
}

int main(int argc, char** argv)
{

}