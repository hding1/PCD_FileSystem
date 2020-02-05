#include <sb.h>
#include <db.h>

void sb_init(){
	sb super;
	super.NUM_BLOCK = 262144;
	super.NUM_FREE_BLOCK = 262011;//??
	super.INODE_PER_BLOCK = 32;
	super.NUM_DATABLOCK; = 262011;//??
	super.NUM_MOUNT_TIME = 0;
	super.MOUNT_TIME = time(NULL);

	super.filesize = 1073741824;//1GB
	super.blocksize = 4096;
	super.MAX_NUM_INODE = 4096;
       	super.START_ILIST = &(add_0 + 3*4096);
	super.START_DATA_BLOCK = &(add_0 + 131*4096);
	super.START_BITMAP = &(add_0+1*4096)
	super.FREE_LIST = &(add_0_+103);	
	
	//char* sb_bytes = static_cast<char*>(static_cast<void*>(&sb));
	
	db *input = malloc(sizeof(db));
	
	memcpy(input->block,&super, sizeof(struct sb)); // dest src size

	disk_write(input, add_0);
	
	free(input);
}

sb* sb_read(){
	db* output = (db*)malloc(sizeof(db));
	disk_read(db, add_0);
	sb* super = (sb*)malloc(sizeof(sb));
	
	memcpy(super, output->block, sizeof(sb));

	free(output);

	return super;
}

void sb_write(sb* super){

        db *input = malloc(sizeof(db));

        memcpy(input->block,&super, sizeof(struct sb)); // dest src size

        disk_write(input, add_0);
        
        free(input);
}
