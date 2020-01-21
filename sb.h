#define NUM_INODE;



struct sb{
	// Blocks in the file system
	int blk_size;
	// No of free blocks in the file system
	int num_free_blk;
	// Inodes per block group
	int max_free_inode;
	int num_free_inode;
	int ilist[NUM_INODE] // free inode list
	// Blocks per block group
	int max_free_blk;
	int num_free_blk;
	int next_free_blk;
	// No of times the file system was mounted since last fsck.

	// Mount time

	// UUID of the file system

	// Write time

	// File System State (ie: was it cleanly unounted, errors detected etc)

	// The file system type etc(ie: whether its ext2,3 or 4).

	// The operating system in which the file system was formatted


}

void sb_init();
int sb_read();
int sb_write();
