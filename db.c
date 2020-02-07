// Responsible author(s): Fuheng Zhao
#include <string.h>
#include <stdio.h>
#include <db.h>
#include <fs.c>
#include <sb.h>

unsigned int db_allocate(){
	// Return the bid of a free block
	// STUB
	return 0;
}

int db_free(unsigned int block_id){

	sb* super = sb_read();
	
	unsigned int temp  =  super->FREE_LIST;
	super->FREE_LIST = block_id;
	super->NUM_FREE_BLOCK+=1;

	sb_write(super);
	free(super);

	db* input = (db*)malloc(sizeof(db));
	memcpy(input->block, temp, sizeof(unsigned int));
	disk_write(input, block_id);
	free(input);

	return 1;
}


int db_read(void* out, unsigned int block_id){
	disk_read(out,block_id);
	return 0; // Return 0 on success
}
int db_write(void* in, unsigned int block_id){
	disk_write(in, block_id);
	return 0; // Return 0 on success
}

void disk_read(void* out, unsigned int block_id){
	db* Disk_Buffer = (db*) ((db*) add_0 + block_id);
	memcpy(out, Disk_Buffer, DB_SIZE);
}
void disk_write(void* in, unsigned int block_id){
	db* Disk_Buffer = (db*) ((db*) add_0 + block_id);
	memcpy(Disk_Buffer, in, DB_SIZE);
}


void db_init(){
	sb* super = sb_read();
	unsigned int START_DATA_BLOCK = super->START_DATA_BLOCK;
	db* input = (db*)malloc(sizeof(db));
	//the last free data block doesnt need to be initialized so size-1
	for(unsigned int i = 0; i < NUM_FREE_BLOCK-1; i++){
		memset(inpit, 0, sizeof(db));
		unsigned int id = i+START_DATA_BLOCK;
		memcpy(input, id+1, sizeof(unsigned int));		
		disk_write(input, id);
	}
	free(input);

}

