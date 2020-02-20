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

	super->FREE_LIST = *new_free_id;
	
	sb_write(super);

	free(super);
	free(buffer);

	return bid;
}

int db_free(unsigned int block_id){

	sb* super = sb_read();
	
	unsigned int temp  =  super->FREE_LIST;
	unsigned int* temp_ptr = &temp;
	super->FREE_LIST = block_id;
	super->NUM_FREE_BLOCK+=1;

	sb_write(super);
	free(super);

	void* input = malloc(DB_SIZE*sizeof(char));
	memcpy(input, temp_ptr, sizeof(unsigned int)); //dest, source
	disk_write(input, block_id);
	free(input);

	return 0;
}

int is_db_free(unsigned int block_id){
	sb* super = sb_read();
	unsigned int F_L = super->FREE_LIST;
	unsigned int F_B = super->NUM_FREE_BLOCK;
	void* buffer = malloc(sizeof(char) * DB_SIZE);
	//printf("is_db_free: ourside: freelist bid = %d, block_id = %d\n", F_L, block_id);
	if(F_L == block_id){
		free(super);
		//printf("is_db_free: freelist bid = %d\n", F_L);
		return 1;
	}
	for(unsigned int i=0; i<F_B;i++){
		disk_read(buffer,F_L);
		memcpy(&F_L,buffer,sizeof(unsigned int));
		if(F_L == block_id){
			//printf("is_db_free: freelist bid = %d\n", F_L);
			free(super);
			return 1;
		}
	}

	free(super);
	return 0; //bid is not free 
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
	
	//the last free data block will contain a last+1 even tho it can not be used.
	
	for(unsigned int i = 0; i < NUM_FREE_BLOCK; i++){
		unsigned int id = i+START_DATA_BLOCK + 1;
		memcpy(input, &id, sizeof(unsigned int));		
		disk_write(input, id-1);
	}
	
	free(input);
	free(super);

}

