#include "sb.h"
#include <string.h>
#include <stdio.h>

int sb_init(){
	sb super;
	super.NUM_BLOCK = 262144;
	super.NUM_FREE_BLOCK = 262013; // 262144 - 3 - 128
	super.INODE_PER_BLOCK = 32;
	super.NUM_DATABLOCK = 262013;//
	super.NUM_MOUNT_TIME = 0;
	super.MOUNT_TIME = time(NULL);

	super.filesize = 1073741824;//1GB
	super.blocksize = 4096;
	super.MAX_NUM_INODE = 4096;
    	super.START_ILIST = 3;
	super.START_DATA_BLOCK = 131;
	super.START_BITMAP = 1;
	super.FREE_LIST = 131;

	super.DIR_ID_NUM = 12;
	super.INDIR_ID_NUM = 1024;
	super.D_INDIR_ID_NUM = 1024*1024;
	super.T_INDIR_ID_NUM = 1024*1024*1024;
	super.INODE_SIZE = 128;
	super.ROOT_INUM = 0;
	
	
	void* input = malloc(sizeof(char) * DB_SIZE);
	
	memcpy(input, &super, sizeof(struct sb)); // dest src size

	if(disk_write(input, 0) == -1){
		free(input);
		return -1;
	}
	
	free(input);
	return 0;
}

int sb_read(sb* super){
	void* output = malloc(sizeof(char) * DB_SIZE);
	
	if(disk_read(output, 0) == -1){
		return -1;
	}
	
	
	memcpy(super, output, sizeof(sb));

	free(output);

	return 0;
}

int sb_write(sb* super){

        void* input = malloc(sizeof(char) * DB_SIZE);

        memcpy(input, super, sizeof(sb)); // dest src size

        if(disk_write(input, 0) == -1){
		return -1;
	}
        
        free(input);
	return 0;
}
