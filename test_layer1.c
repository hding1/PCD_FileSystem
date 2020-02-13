//author: Fuheng，Dennis
#include "sb.h"
#include "db.h"
#include "disk.h"
#include "inode.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define PASS 0
#define FAIL -1


/*************************************inode test helpers***************************************/
int test_bitmap_init(){
	int status = inode_bitmap_init();
	if(status){
		printf("Error: bitmap init failed!\n");
		return FAIL;
	}
	char block[4096];
	unsigned int bitmap[1024];
	db_read(block, 1);
	memcpy(bitmap, block, 4096);
	for(int i = 0; i < 1024; i++){
		if(bitmap[i] != 0){
			printf("Error: bitmap uninitialized at index %d !\n",i);
			return FAIL;
		}
	}
	return PASS;
}

int test_inode_list_init(){
	int status = inode_list_init();
	if(status){
		printf("Error: inode list init failed!\n");
		return FAIL;
	}
	char block[4096];
	inode node;
	db_read(block,2);
	memcpy(&node, block, sizeof(inode));
	if(node.mode != 0 || node.size != 0 || node.link_count != 0){
		printf("Error: FIRST inode in ilist uninitialized!\n");
		return FAIL;
	}
	db_read(block,129);
	memcpy(&node, block + 128 * 31, sizeof(inode));
	if(node.mode != 0 || node.size != 0 || node.link_count != 0){
		printf("Error: LAST inode in ilist uninitialized!\n");
		return FAIL;
	}
	return PASS;
}


/***************************************test main********************************************/
int main(){
	
	allocate_disk();
	
	//sb tests
	sb_init();
	sb* super = sb_read();
	assert(super->NUM_BLOCK == 262144);
	assert(super->blocksize == 4096);
	assert(super->filesize == 1073741824);
	assert(super->FREE_LIST == 130);

	super->NUM_BLOCK-=1;
	unsigned int new_block = super->NUM_BLOCK;	
	sb_write(super);
	super = sb_read();
	assert(super->NUM_BLOCK == new_block);

	//db tests
	db_init();
	assert(db_allocate() == super->FREE_LIST);
	
	new_block = super->FREE_LIST+1;
	
	super = sb_read();
	printf("%d \n",super->FREE_LIST);
	assert(super->FREE_LIST == new_block);

	db_free(new_block-1);

	super=sb_read();
	assert(super->FREE_LIST == new_block-1);
		
	free(super);



	/*---inode tests---*/

	printf("--------Running test 1: test_bitmap_init!--------\n");
	if(!test_bitmap_init()){
		printf("PASS: test_bitmap_init!\n");
	}else{
		printf("FAIL: test_bitmap_init\n");
	}

	printf("--------Running test 2: test_inode_list_init!--------\n");
	if(!test_inode_list_init()){
		printf("PASS: test_inode_list_init!\n");
	}else{
		printf("FAIL: test_inode_list_init!\n");
	}



	free_disk();
	return 0;
}
