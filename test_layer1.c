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

int test_inode_allocate(){
	inode* node;
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}
	node = find_inode_by_inum(inum);
	if(node->link_count != 1){
		printf("Error: a single inode incorrectly initialized!\n");
		free(node);
		return FAIL;
	}else{
		free(node);
	}

	int inums[32];
	for(int i = 0; i < 32; i++){
		inums[i] = inode_allocate();
	}
	for(int i = 0; i < 32; i++){
		node = find_inode_by_inum(inums[i]);
		if(node->link_count != 1){
			printf("Error: a block of inode incorrectly initialized!\n");
			free(node);
			return FAIL;
		}else{
			free(node);
		}
	}

	inode_free(inum);
	int recycle = inode_allocate();
	if(recycle != inum){
		printf("Error: freed inode recycle failed!\n");
		return FAIL;
	}

	inode_free(recycle);
	for(int i = 0; i < 32; i++){
		inode_free(inums[i]);
	}

	return PASS;
}

int test_inode_free(){
	unsigned long numbs = 12 + 1024 + 1024*2;
	unsigned int dbs[numbs];
	int bid;

	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}

	for(unsigned long i = 0; i < numbs; i++){
		bid = add_block(inum);
		if(bid == -1){
			printf("Error: inode add block failed!\n");
			return FAIL;
		}
		dbs[i] = bid;
	}

	inode_free(inum);

	// check if data blocks are freed
	for(unsigned long i = 0; i < numbs; i++){
		if(!is_db_free(dbs[i])) return FAIL;
	}
	// check if bitmap is freed
	unsigned short bitmap[4096];
	char block[4096];
	db_read(block, 1);
	memcpy(bitmap, block, 4096);
	if(bitmap[inum] != 0){
		printf("Error: bitmap free failed!\n");
		return FAIL;
	}

	return PASS;
}

int test_inode_mode_read_write(){
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}

	mode_t mode_out;
	if(inode_read_mode(inum, &mode_out)){
		printf("Error: inode read mode failed!\n");
		return FAIL;
	}
	if(mode_out != 0666){
		printf("Error: inode read mode incorrectly!\n");
		return FAIL;
	}
	mode_t mode_in = 0777;
	if(inode_write_mode(inum, mode_in)){
		printf("Error: inode write mode failed!\n");
		return FAIL;
	}
	if(inode_read_mode(inum, &mode_out)){
		printf("Error: inode read mode failed!\n");
		return FAIL;
	}
	if(mode_out != 0777){
		printf("Error: inode write mode incorrectly!\n");
		return FAIL;
	}

	return PASS;
}

int test_inode_link_read_reduce(){
	return PASS;
}

int test_inode_read_size(){
	return PASS;
}

int test_inode_rootnum(){
	if(get_root_inum() != 0) return FAIL;
	return PASS;
}

int test_inode_read_file(){
	return PASS;
}

int test_inode_write_file(){
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

	printf("--------Running test 3: test_inode_allocate!--------\n");
	if(!test_inode_allocate()){
		printf("PASS: test_inode_allocate!\n");
	}else{
		printf("FAIL: test_inode_allocate!\n");
	}

	printf("--------Running test 4: test_inode_free!--------\n");
	if(!test_inode_free()){
		printf("PASS: test_inode_free!\n");
	}else{
		printf("FAIL: test_inode_free!\n");
	}

	printf("--------Running test 5: test_inode_mode_read_write!--------\n");
	if(!test_inode_mode_read_write()){
		printf("PASS: test_inode_mode_read_write!\n");
	}else{
		printf("FAIL: test_inode_mode_read_write!\n");
	}

	printf("--------Running test 6: test_inode_link_read_reduce!--------\n");
	if(!test_inode_link_read_reduce()){
		printf("PASS: test_inode_link_read_reduce!\n");
	}else{
		printf("FAIL: test_inode_link_read_reduce!\n");
	}

	printf("--------Running test 7: test_inode_read_size!--------\n");
	if(!test_inode_read_size()){
		printf("PASS: test_inode_read_size!\n");
	}else{
		printf("FAIL: test_inode_read_size!\n");
	}

	printf("--------Running test 8: test_inode_rootnum!--------\n");
	if(!test_inode_rootnum()){
		printf("PASS: test_inode_rootnum!\n");
	}else{
		printf("FAIL: test_inode_rootnum!\n");
	}

	printf("--------Running test 9: test_inode_read_file!--------\n");
	if(!test_inode_read_file()){
		printf("PASS: test_inode_read_file!\n");
	}else{
		printf("FAIL: test_inode_read_file!\n");
	}

	printf("--------Running test 10: test_inode_write_file!--------\n");
	if(!test_inode_write_file()){
		printf("PASS: test_inode_write_file!\n");
	}else{
		printf("FAIL: test_inode_write_file!\n");
	}

	free_disk();
	return 0;
}
