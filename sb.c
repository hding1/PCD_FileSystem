#include "sb.h"
#include <string.h>
#include <stdio.h>

int sb_init(){
	sb super;
	super.NUM_BLOCK = 7864320;
	super.NUM_FREE_BLOCK = 7863029; // NUM_BLOCK - 1 - 10 - 1280
	super.INODE_PER_BLOCK = 32;
	super.NUM_DATABLOCK = 7863029;//
	super.NUM_MOUNT_TIME = 0;
	super.MOUNT_TIME = time(NULL);

	super.START_BITMAP = 1;
	super.START_ILIST = 11;		// 1 + 10
	super.START_DATA_BLOCK = 1291;	// 11 + 128*10

	super.filesize = (unsigned long)30 * 1024 * 1024 * 1024;
	super.MAX_FILE_SIZE = (12+1024+1024*1024+(unsigned long)1024*1024*1024)*4096;
	super.blocksize = 4096;
	super.MAX_NUM_INODE = 4096 * 10;

	super.FREE_LIST = 1291;

	super.DIR_ID_NUM = 12;
	super.INDIR_ID_NUM = 1024;
	super.D_INDIR_ID_NUM = 1024*1024;
	super.T_INDIR_ID_NUM = 1024*1024*1024;
	super.INODE_SIZE = 128;
	super.ROOT_INUM = 0;
	
	
	void* input = malloc(sizeof(char) * DB_SIZE);
	
	memcpy(input, &super, sizeof(struct sb)); // dest src size

	if(disk_write(input, 0, 1) == -1){
		free(input);
		return -1;
	}
	
	free(input);
	return 0;
}

int sb_read(sb* super){
	// void* output = malloc(sizeof(char) * DB_SIZE);
	char output[DB_SIZE];
	
	if(disk_read(output, 0) == -1){
		return -1;
	}
	
	if(sizeof(sb) > DB_SIZE){
		printf("sb size larger than DB_SIZE\n");
		return -1;
	}
	
	memcpy(super, output, sizeof(sb));

	// free(output);

	return 0;
}

int sb_write(sb* super){

        void* input = malloc(sizeof(char) * DB_SIZE);

        memcpy(input, super, sizeof(sb)); // dest src size

        if(disk_write(input, 0, 0) == -1){
		return -1;
	}
        
        free(input);
	return 0;
}
