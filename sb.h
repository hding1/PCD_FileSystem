// Responsible author(s): Fuheng

#include <stdlib.h>
#include <time.h>
#include "disk.h"
#define DB_SIZE 4096

#ifndef SB_H_
#define SB_H_
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
}sb;


void sb_init();
sb* sb_read();
void sb_write();

#endif
