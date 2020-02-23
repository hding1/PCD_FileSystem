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
	unsigned long numbs = 12 + 1024 + 1024 * 2;
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
		// Simulate file write, increase size
		inode* target_node = find_inode_by_inum(inum);
		target_node->size += 4096;
		write_inode_to_disk(inum,target_node);

		dbs[i] = bid;
		//printf("test_inode_free: bid%lu = %d\n", i, dbs[i]);
	}

	inode_free(inum);

	// check if data blocks are freed
	for(unsigned long i = 0; i < numbs; i++){
		if(!is_db_free(dbs[i])){
			printf("Error: Used data block is not freed!\n");
			return FAIL;
		}
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
	if(inode_read_mode(inum, &mode_out) == -1){
		printf("Error: inode read mode failed!\n");
		return FAIL;
	}
	if(mode_out != 0666){
		printf("Error: inode read mode incorrectly!\n");
		return FAIL;
	}
	mode_t mode_in = 0777;
	if(inode_write_mode(inum, mode_in) == -1){
		printf("Error: inode write mode failed!\n");
		return FAIL;
	}
	if(inode_read_mode(inum, &mode_out) == -1){
		printf("Error: inode read mode failed!\n");
		return FAIL;
	}
	if(mode_out != 0777){
		printf("Error: inode write mode incorrectly!\n");
		return FAIL;
	}
	inode_free(inum);
	return PASS;
}

int test_inode_link_read_reduce(){
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}
	unsigned int lc;
	if(inode_read_link_count(inum, &lc) == -1){
		printf("Error: inode read link count failed!\n");
		return FAIL;
	}
	if(lc != 1){
		printf("Error: inode read link count incorrectly!\n");
		return FAIL;
	}
	if(inode_reduce_link_count(inum) == -1){
		printf("Error: inode reduce link count failed!\n");
		return FAIL;
	}
	inode* target = find_inode_by_inum(inum);
	if(target != NULL){
		printf("Error: inode reduce link count incorrectly!\n");
		return FAIL;
	}
	return PASS;
}

int test_inode_read_size(){
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}
	set_inode_size(inum, 100);
	unsigned long mysize;
	if(inode_read_size(inum, &mysize) == -1){
		printf("Error: inode read size failed!\n");
		return FAIL;
	}
	if(mysize != 100){
		printf("Error: inode read size incorrectly!\n");
		return FAIL;
	}
	set_inode_size(inum, 0);
	inode_free(inum);
	return PASS;
}

int test_inode_rootnum(){
	if(get_root_inum() != 0) return FAIL;
	return PASS;
}

int test_inode_read_file(){
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}
	// read one direct block
	char wbuf[4096];
	char rbuf[4096];
	for(int i = 0; i < 4096; i++){
		wbuf[i] = 'A';
	}
	if(write_file(inum, wbuf, 4096, 0) == -1){
		printf("Error: write file failed!\n");
		return FAIL;
	}
	if(read_file(inum, rbuf, 4096, 0) == -1){
		printf("Error: read file failed!\n");
		return FAIL;
	}
	for(int i = 0; i < 4096; i++){
		if(rbuf[i] != 'A'){
			printf("Error: rbuf read file one direct block incorrectly!\n");
			return FAIL;
		}
	}
	// read one direct block, buffer larger than inode_size
	char lrbuf[6000];
	int readsize = read_file(inum, lrbuf, 6000, 0);
	if(readsize == -1){
		printf("Error: read file failed!\n");
		return FAIL;
	}
	// printf("test read file: inum = %d\n", inum);
	// printf("test read file: inode size = %lu\n", get_inode_size(inum));
	// printf("test read file: readsize = %d\n", readsize);
	// printf("test read file: lrbuf = %s\n", lrbuf);
	if(readsize != 4096){
		printf("Error: read file size exceed!\n");
		return FAIL;
	}
	for(int i = 0; i < 4096; i++){
		if(lrbuf[i] != 'A'){
			printf("Error: lrbuf read file one direct block incorrectly!\n");
			return FAIL;
		}
	}
	// read all direct blocks
	char allwbuf[4096 * 12];
	char allrbuf[4096 * 12];
	for(int i = 0; i < 4096*12; i++){
		allwbuf[i] = 'A';
	}
	if(write_file(inum, allwbuf, 4096*12, 0) == -1){
		printf("Error: write file failed!\n");
		return FAIL;
	}
	readsize = read_file(inum, allrbuf, 4096*12, 0);
	if( readsize != 4096 * 12){
		printf("Error: read file failed!\n");
		return FAIL;
	}
	// printf("test read file: inum = %d\n", inum);
	// printf("test read file: inode size = %lu\n", get_inode_size(inum));
	// printf("test read file: readsize = %d\n", readsize);
	// printf("test read file: allrbuf = %s\n", allrbuf);
	for(int i = 0; i < 4096*12; i++){
		if(allrbuf[i] != 'A'){
			printf("Error: read file all direct blocks incorrectly!\n");
			return FAIL;
		}
	}
	// read single indir blocks

	// read double indir blocks

	inode_free(inum);
	return PASS;
}

int test_inode_write_file_direct_blo(){
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}

	/**********Test write direct block**********/
	unsigned int bufA_size = 4096 * 12;
	unsigned int bufB_size = 2;
	char bufA[bufA_size];
	char bufB[bufB_size];
	for(int i = 0; i < bufA_size; i++){
		bufA[i] = 'A';
	}
	for(int i  = 0; i < bufB_size; i++){
		bufB[i] = 'B';
	}
	// Test with buffer A (write all direct blocks)
	if(write_file(inum, bufA, bufA_size, 0) == -1){	
		printf("Error: write file failed!\n");
		return FAIL;
	}
	inode* target = find_inode_by_inum(inum);
	int bid;
	char result[4096];
	for(int i = 0; i < 12; i++){
		bid = target->direct_blo[i];
		db_read(result, bid);
		//printf("test bufA: block%d = %s\n", i, result);
		for(int j = 0; j < 4096; j++){
			if(result[j] != 'A'){
				printf("Error: bufA write file db%d incorrectly\n", i);
				return FAIL;
			}
		}
	}
	// Test with buffer B (write between existed blocks with offset)
	if(write_file(inum, bufB, bufB_size, 4095) == -1){	
		printf("Error: write file failed!\n");
		return FAIL;
	}
	target = find_inode_by_inum(inum);
	int bid0 = target->direct_blo[0];
	int bid1 = target->direct_blo[1];
	char r0[4096];
	char r1[4096];
	db_read(r0, bid0);
	db_read(r1, bid1);
	if(r0[4095] != 'B' || r1[0] != 'B'){
		printf("Error: bufB write file incorrectly\n");
		return FAIL;
	}
	// Test with buffer C (partially write on existed blocks, partially write on new blocks. file size extended)
	int newinum = inode_allocate();
	if(newinum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}
	//printf("test bufCD: newinum = %d\n", newinum);
	char bufC[4096];
	char bufD[4096 * 2];
	for(int i = 0; i < 4096; i++){
		bufC[i] = 'C';
	}
	for(int i = 0; i < 4096 * 2; i++){
		bufD[i] = 'D';
	}
	//printf("test bufCD: writing bufC!\n");
	if(write_file(newinum, bufC, 4096, 0) == -1){	
		printf("Error: write file failed!\n");
		return FAIL;
	}
	//printf("test bufCD: writing bufD!\n");
	if(write_file(newinum, bufD, 4096*2, 0) == -1){	
		printf("Error: write file failed!\n");
		return FAIL;
	}
	target = find_inode_by_inum(newinum);
	int newbid0 = target->direct_blo[0];
	int newbid1 = target->direct_blo[1];
	char newr0[4096];
	char newr1[4096];
	//printf("test bufCD: bid0 = %d\n", newbid0);
	//printf("test bufCD: bid1 = %d\n", newbid1);
	db_read(newr0, newbid0);
	db_read(newr1, newbid1);
	//printf("test bufCD: block%d = %s\n", 0, newr0);
	//printf("test bufCD: block%d = %s\n", 1, newr1);
	for(int i = 0; i < 4096; i++){
		if(newr0[i] != 'D' || newr1[i] != 'D'){
			printf("Error: bufCD write file incorrectly\n");
			return FAIL;
		}
	}

	inode_free(inum);
	inode_free(newinum);
	free(target);
	return PASS;
}

int test_inode_write_file_single_indirect_blo(){
	return PASS;
}

int test_inode_write_file_double_indirect_blo(){
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
	//printf("%d \n",super->FREE_LIST);
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

	printf("--------Running test 10a: test_inode_write_file_direct_blo!--------\n");
	if(!test_inode_write_file_direct_blo()){
		printf("PASS: test_inode_write_file_direct_blo!\n");
	}else{
		printf("FAIL: test_inode_write_file_direct_blo!\n");
	}

	printf("--------Running test 10b: test_inode_write_file_single_indirect_blo!--------\n");
	if(!test_inode_write_file_single_indirect_blo()){
		printf("PASS: test_inode_write_file_single_indirect_blo!\n");
	}else{
		printf("FAIL: test_inode_write_file_single_indirect_blo!\n");
	}

	printf("--------Running test 10c: test_inode_write_file_double_indirect_blo!--------\n");
	if(!test_inode_write_file_double_indirect_blo()){
		printf("PASS: test_inode_write_file_double_indirect_blo!\n");
	}else{
		printf("FAIL: test_inode_write_file_double_indirect_blo!\n");
	}

	free_disk();
	return 0;
}
