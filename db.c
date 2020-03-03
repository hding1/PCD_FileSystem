// Responsible author(s): Fuheng Zhao
#include <string.h>
#include <stdio.h>
#include "db.h"


int db_allocate(){
	// Return the bid of a free block
	sb* super = (sb*)malloc(sizeof(sb));

	if(sb_read(super)==-1){
		free(super);
		return -1;
	}
	unsigned int bid  =  super->FREE_LIST;
	if(super->NUM_FREE_BLOCK==0){
		printf("no free block left \n");
		free(super);
		return -1;
	}
	else{	
		super->NUM_FREE_BLOCK-=1;
	}
	void* buffer = malloc(sizeof(char) * DB_SIZE);
	
	if(disk_read(buffer,bid) == -1){
		return -1;
	}

	if(bid>super->NUM_BLOCK){
		printf("bid %u too large in allocation\n",bid);
                bid=-1;
		super->NUM_FREE_BLOCK =0;
		if(sb_write(super) == -1){
			bid = -1;
		}
		free(super);
		free(buffer);
		return bid;
	}
	unsigned int* new_free_id = malloc(sizeof(unsigned int));
	memcpy(new_free_id, buffer, sizeof(unsigned int));
	if(*new_free_id < super->START_DATA_BLOCK){
		printf("bid = %d\n", *new_free_id);
		printf("db allocated a bid thats too samll \n");
		return -1;
	}
	super->FREE_LIST = *new_free_id;	
	if(sb_write(super) == -1){
		printf("update super in db allocation failed \n");
		bid = -1;
	}
	free(super);
	free(buffer);
	return bid;
}

int db_free(unsigned int block_id){

        sb* super = (sb*)malloc(sizeof(sb));
        if(sb_read(super)==-1){
                free(super);
                return -1;
        }
	
	unsigned int temp  =  super->FREE_LIST;
	unsigned int* temp_ptr = &temp;
	super->FREE_LIST = block_id;
	super->NUM_FREE_BLOCK+=1;

	if(sb_write(super) == -1){
		free(super);
		return -1;
	}
	free(super);

	void* input = malloc(DB_SIZE*sizeof(char));
	memcpy(input, temp_ptr, sizeof(unsigned int)); //dest, source
	disk_write(input, block_id);
	free(input);

	return 0;
}

int is_db_free(unsigned int block_id){
	sb* super = (sb*)malloc(sizeof(sb));
        if(sb_read(super)==-1){
                free(super);
                return -1;
        }

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
        sb* super = (sb*)malloc(sizeof(sb));
        if(sb_read(super)==-1){
                free(super);
                return -1;
        }

	if(block_id > super->NUM_BLOCK){
		free(super);
		printf("block id %u too large to read \n",block_id);
		return -1;		
	}
	if(disk_read(out,block_id) == -1){
		return -1;
	}
	free(super);
	return 0; // Return 0 on success
}
int db_write(void* in, unsigned int block_id){
	sb* super = (sb*)malloc(sizeof(sb));
	if(sb_read(super) == -1){
		free(super);
		return -1;
	}
	if(block_id > super->NUM_BLOCK){
		free(super);
		printf("block %u id too large to write \n", block_id);
		return -1;
	}
	if(disk_write(in, block_id) == -1){
		return -1;
	}
	free(super);
	return 0; // Return 0 on success
}
int db_init(){
	sb* super = (sb*)malloc(sizeof(sb));
	if(sb_read(super) == -1){
		free(super);
		return -1;	
	}
	unsigned int START_DATA_BLOCK = super->START_DATA_BLOCK;
	void* input = malloc(sizeof(char)*DB_SIZE);
	unsigned int NUM_FREE_BLOCK = super->NUM_FREE_BLOCK;
	
	//the last free data block will contain a last+1 even tho it can not be used.
	
	for(unsigned int i = 0; i < NUM_FREE_BLOCK; i++){
		unsigned int id = i+START_DATA_BLOCK + 1;
		memcpy(input, &id, sizeof(unsigned int));		
		if(disk_write(input, id-1) == -1){
			free(input);
			free(super);
			return -1;
		
		}
	}
	
	free(input);
	free(super);
	return 0;

}

