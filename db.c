// Responsible author(s): Fuheng Zhao
#include <string.h>
#include <stdio.h>
#include "db.h"

unsigned int db_allocate(){
	// Return the bid of a free block
	
	struct sb* super = sb_read();
	unsigned int bid  =  super->FREE_LIST;
	if(super->NUM_FREE_BLOCK==0){
		free(super);
		return -1;
	}
	else{	
		super->NUM_FREE_BLOCK-=1;
	}
	void* buffer = malloc(sizeof(char) * DB_SIZE);
	disk_read(buffer,bid);
	unsigned int* new_free_id = malloc(sizeof(unsigned int));
	memcpy(new_free_id, buffer, sizeof(unsigned int));

	super->FREE_LIST = new_free_id;
	
	sb_write(super);

	free(super);
	free(buffer);

	return bid;
}

int db_free(unsigned int block_id){

	sb* super = sb_read();
	
	unsigned int temp  =  super->FREE_LIST;
	super->FREE_LIST = block_id;
	super->NUM_FREE_BLOCK+=1;

	sb_write(super);
	free(super);

	void* input = malloc(DB_SIZE*sizeof(char));
	memcpy(input, temp, sizeof(unsigned int)); //dest, source
	disk_write(input, block_id);
	free(input);

	return 0;
}


int db_read(void* out, unsigned int block_id){
	disk_read(out,block_id);
	return 0; // Return 0 on success
}
int db_write(void* in, unsigned int block_id){
	disk_write(in, block_id);
	return 0; // Return 0 on success
}

/*
void disk_read(void* out, unsigned int block_id){
	void* Disk_Buffer = (void*)((char*)add_0 + block_id*4096);
	memcpy(out, Disk_Buffer, DB_SIZE);
}
void disk_write(void* in, unsigned int block_id){
	void* Disk_Buffer = (void*)((char*)add_0 + block_id*4096);
	memcpy(Disk_Buffer, in, DB_SIZE);
}
*/

void db_init(){
	sb* super = sb_read();
	unsigned int START_DATA_BLOCK = super->START_DATA_BLOCK;
	void* input = malloc(sizeof(unsigned int));
	unsigned int NUM_FREE_BLOCK = super->NUM_FREE_BLOCK;
	//the last free data block doesnt need to be initialized so size-1
	for(unsigned int i = 0; i < NUM_FREE_BLOCK-1; i++){
		unsigned int id = i+START_DATA_BLOCK;
		memcpy(input, id+1, sizeof(unsigned int));		
		disk_write(input, id);
	}
	free(input);
	free(super);

}

