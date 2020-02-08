#include <sb.h>
#include <db.h>

void sb_init(){
	sb super;
	super.NUM_BLOCK = 262144;
	super.NUM_FREE_BLOCK = 262013; // 262144 - 3 - 128
	super.INODE_PER_BLOCK = 32;
	super.NUM_DATABLOCK; = 262013;//
	super.NUM_MOUNT_TIME = 0;
	super.MOUNT_TIME = time(NULL);

	super.filesize = 1073741824;//1GB
	super.blocksize = 4096;
	super.MAX_NUM_INODE = 4096;
       	super.START_ILIST = 3;
	super.START_DATA_BLOCK = 130;
	super.START_BITMAP = 1;
	super.FREE_LIST = 130;	
	
	
	void* input = malloc(sizeof(char) * DB_SIZE);
	
	memcpy(input, &super, sizeof(struct sb)); // dest src size

	disk_write(input, 0);
	
	free(input);
}

sb* sb_read(){
	void* output = malloc(sizeof(char) * DB_SIZE);
	
	disk_read(output, 0);
	
	sb* super = (sb*)malloc(sizeof(sb));
	
	memcpy(super, output, sizeof(sb));

	free(output);

	return super;
}

void sb_write(sb* super){

        void* input = malloc(sizeof(char) * 4096);

        memcpy(input, super, sizeof(sb)); // dest src size

        disk_write(input, 0);
        
        free(input);
}
