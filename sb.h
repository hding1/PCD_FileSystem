// Responsible author(s): DZ

#include <stdlib.h>
#include <inode.h>
#include <db.h>

#define NUM_INODE 4096

typedef struct sb{
	// Blocks in the file system

	// No of free blocks in the file system
	
	// Inodes per block group
	// Blocks per block group
	// No of times the file system was mounted since last fsck.
	// Mount time
	// UUID of the file system
	// Write time
	// File System State (ie: was it cleanly unounted, errors detected etc)
	// The file system type etc(ie: whether its ext2,3 or 4).
	// The operating system in which the file system was formatted

	unsigned int filesize;
	unsigned int blocksize;
	unsigned int MAX_NUM_INODE;
	void* ilist;
	void* data_block;
	void* bitmap;
	void* free_list;
}sb;


void sb_init();
int sb_read();
int sb_write();
