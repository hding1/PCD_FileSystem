// Responsible author(s): DZ

#include <stdlib.h>
#include <inode.h>
#include <db.h>

#define NUM_INODE 4096

typedef struct sb{
	// Blocks in the file system
	unsigned int NUM_BLOCK;
	// No of free blocks in the file system
	unsigned int NUM_FREE_BLOCK;
	// Inodes per block group
	unsigned int INODE_PER_BLOCK;
	// Blocks per block group
	unsigned int NUM_DATABLOCK;
	// No of times the file system was mounted since last fsck.
	unsigned int NUM_MOUNT_TIME;
	// Mount time
	time_t MOUNT_TIME;
	// UUID of the file system
	// Write time
	// File System State (ie: was it cleanly unounted, errors detected etc)
	// The file system type etc(ie: whether its ext2,3 or 4).
	// The operating system in which the file system was formatted

	unsigned int filesize;
	unsigned int blocksize;
	unsigned int MAX_NUM_INODE;
	unsigned int START_DATA_BLOCK;
	unsigned int START_ILIST;
	unsigned int START_BITMAP;
	unsigned int FREE_LIST;
};


void sb_init();
int sb_read();
int sb_write();
