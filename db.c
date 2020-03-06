// Responsible author(s): Fuheng Zhao
#include "db.h"


int db_allocate(){
	// Return the bid of a free block
	sb* super = (sb*)malloc(sizeof(sb));

	if(sb_read(super)==-1){
		free(super);
		return -1;
	}
	unsigned int bid  =  super->FREE_LIST;
	if(super->NUM_FREE_BLOCK==0 || bid == UINT_MAX){
		if(super->FREE_LIST != UINT_MAX){
			printf("free list bid is wrong \n");
			super->FREE_LIST = UINT_MAX;
			sb_write(super);
		}
		if(super->NUM_FREE_BLOCK != 0){
                        printf("num free block is wrong \n");
                        super->NUM_FREE_BLOCK = 0;
                        sb_write(super);
                }
		printf("no free block left \n");
		free(super);
		return -1;
	}
	else{	
		super->NUM_FREE_BLOCK-=1;
	}
	void* buffer = calloc(sizeof(char) * DB_SIZE,1);
	
	if(disk_read(buffer,bid) == -1){
		return -1;
	}
	/*
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
	}*/

	unsigned int* new_free_id = malloc(sizeof(unsigned int));
	
	/*memcpy(new_free_id, buffer, sizeof(unsigned int));
	if(*new_free_id < super->START_DATA_BLOCK){
		printf("bid = %d\n", *new_free_id);
		printf("db allocated a bid thats too samll \n");
		bid = -1;
	}
	super->FREE_LIST = *new_free_id;	
	if(sb_write(super) == -1){
		printf("update super in db allocation failed \n");
		bid = -1;
	}
	free(super);
	free(buffer);
	free(new_free_id);
	return bid;*/

	unsigned int MAX_ENTRY = DB_SIZE/sizeof(unsigned int);
	unsigned int i;
	for(i=1; i<MAX_ENTRY;i++){
		memcpy(new_free_id, ((char*)buffer)+i*sizeof(unsigned int), sizeof(unsigned int));
		if(*new_free_id != 0){
			// we found a free spot;
			bid = *new_free_id;
			unsigned int t = 0;
			memcpy(((char*)buffer) + i*sizeof(unsigned int), &t, sizeof(unsigned int));
			if(disk_write(buffer, bid)==-1){
				bid = -1;
			}
			if(sb_write(super) == -1){
				bid = -1;
			}
			if(bid>super->NUM_BLOCK){
				printf("bid too large \n");
				exit(0);
			}
			free(super);
			free(buffer);
			free(new_free_id);
			
			return bid;
		}	
	}
	// now all recorded data blocks are used therefore it become a new data block;
	memcpy(new_free_id, (buffer), sizeof(unsigned int));
	if(*new_free_id == bid){
		super->NUM_FREE_BLOCK = 0;
		super->FREE_LIST = UINT_MAX;
	}
	else{
		super->FREE_LIST = *new_free_id;
	}
	sb_write(super);
	if(bid>super->NUM_BLOCK){
		printf("bid too large \n");
		exit(0);
	}
	free(super);
	free(buffer);
	free(new_free_id);
	return bid;
}

int db_free(unsigned int block_id){

        sb* super = (sb*)malloc(sizeof(sb));
        if(sb_read(super)==-1){
                free(super);
                return -1;
        }
	
	/*unsigned int temp  =  super->FREE_LIST;
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
	*/
	void* buffer = calloc(DB_SIZE*sizeof(char),1);
	unsigned int bid  =  super->FREE_LIST;
	if(bid == UINT_MAX || super->NUM_FREE_BLOCK == 0){
		super->NUM_FREE_BLOCK = 1;
		super->FREE_LIST = block_id;
		sb_write(super);
		free(super);

		memcpy(buffer, &block_id, sizeof(unsigned int));
		disk_write(buffer, block_id);
		free(buffer);
		return 0;
	}
	disk_read(buffer,bid);
	unsigned int MAX_ENTRY = DB_SIZE/sizeof(unsigned int);
	for(unsigned int i = 1; i<MAX_ENTRY;i++){
		unsigned int* temp = malloc(sizeof(unsigned int));
		memcpy(temp, ((char*)buffer)+i*sizeof(unsigned int), sizeof(unsigned int));
		if(*temp == 0){
			//we found a free entry
			memcpy(((char*)buffer) + i*sizeof(unsigned int), &block_id, sizeof(unsigned int));
			disk_write(buffer,bid);
			free(buffer);
			super->NUM_FREE_BLOCK +=1;
			sb_write(super);
			free(super);

			free(temp);
			return 0;
		}
	}	
	//this block is full
	//block_id block becomes the new freeblock;
	void* new_free_block = calloc(DB_SIZE*sizeof(char),1);
	memcpy(new_free_block,&bid,sizeof(unsigned int));
	super->FREE_LIST = block_id;
	super->NUM_FREE_BLOCK +=1;
	if(sb_write(super) == -1){
		return -1;
	}
	if(disk_write(new_free_block,block_id)==-1){
		return -1;
	}
	free(buffer);
	free(super);
	free(new_free_block);
	return 0;
}
/*
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
}*/

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
	/*
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
	*/

	unsigned int START_DATA_BLOCK = super->START_DATA_BLOCK;
        void* input = malloc(sizeof(char)*DB_SIZE);
	unsigned int NUM_FREE_BLOCK = super->NUM_FREE_BLOCK;
	unsigned int MAX_ENTRY = DB_SIZE/sizeof(unsigned int);
	unsigned int MAX_INDEX = NUM_FREE_BLOCK / MAX_ENTRY;
	for(unsigned int i=0; i<MAX_INDEX;i++){
		unsigned int id = (i+1)*MAX_ENTRY;
		memcpy((input), &id, sizeof(unsigned int));
		for(unsigned int j=1; j<MAX_ENTRY;j++){
			id = START_DATA_BLOCK+i*MAX_ENTRY+j;
			memcpy(((char*)input)+j*sizeof(unsigned int), &id, sizeof(unsigned int));
		}
		if(disk_write(input, START_DATA_BLOCK + i*MAX_ENTRY)==-1){
			free(input);
			free(super);
			return -1;
		}
	}
	int any_left = NUM_FREE_BLOCK - MAX_INDEX * MAX_ENTRY;
	free(input);
	input = calloc(sizeof(char)*DB_SIZE, 1); 
	if(any_left > 0){
		//there are a few blocks left
		for(unsigned int i = 0; i<any_left; i++){
			unsigned int id = START_DATA_BLOCK + MAX_INDEX * MAX_ENTRY + i;
			memcpy(((char*)input)+i*sizeof(unsigned int), &id, sizeof(unsigned int));
		}
		if(disk_write(input, START_DATA_BLOCK + MAX_INDEX * MAX_ENTRY)==-1){
                        free(input);
                        free(super);
                        return -1;
                }
	}
	free(input);
	free(super);
	return 0;

}

