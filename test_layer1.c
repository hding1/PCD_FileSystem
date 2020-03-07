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
	db_read(block,3);
	memcpy(&node, block, sizeof(inode));
	if(node.mode != 0 || node.size != 0 || node.link_count != 0){
		printf("Error: FIRST inode in ilist uninitialized!\n");
		return FAIL;
	}
	db_read(block,130);
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
	unsigned long numbs = 12 + 1024 + 1024*250;
	//unsigned int dbs[numbs];
	int bid;

	sb super;
	sb_read(&super);
	int numfree = super.NUM_FREE_BLOCK;
	//printf("numfree before add = %d\n", numfree);

	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}

	for(unsigned long i = 0; i < numbs; i++){
		//printf("adding %lu block\n", i);
		bid = add_block(inum);
		if(bid == -1){
			printf("Error: inode add block failed!\n");
			return FAIL;
		}
		// Simulate file write, increase size
		inode* target_node = find_inode_by_inum(inum);
		target_node->size += 4096;
		write_inode_to_disk(inum,target_node);

		//dbs[i] = bid;
		//printf("test_inode_free: bid%lu = %d\n", i, dbs[i]);
	}

	sb_read(&super);
	numfree = super.NUM_FREE_BLOCK;
	//printf("numfree after add = %d\n", numfree);

	inode_free(inum);

	sb_read(&super);
	numfree = super.NUM_FREE_BLOCK;
	//printf("numfree after free = %d\n", numfree);

	// check if data blocks are freed
	// for(unsigned long i = 0; i < numbs; i++){
		// Tested, passed, now commented due to change of structure of freelist

		// if(!is_db_free(dbs[i])){
		// 	printf("Error: Used data block is not freed!\n");
		// 	return FAIL;
		// }
	//}

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

int test_inode_read_file_direct_blo(){
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

	inode_free(inum);
	return PASS;
}

int test_inode_read_file_single_indirect_blo(){
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}
	//printf("111\n");
	int buf_size = (12 + 1024) * 4096;
	char buf[buf_size];
	for(int i = 0; i < buf_size; i++){
		buf[i] = 'A';
	}
	//printf("222\n");
	if(write_file(inum, buf, buf_size, 0) == -1){
		printf("Error: write file failed!\n");
		return FAIL;
	}
	//printf("333\n");
	int readsize = read_file(inum, buf, buf_size, 0);
	if( readsize != buf_size){
		printf("Error: read file readsize incorrect!\n");
		return FAIL;
	}
	for(int i = 0; i < buf_size; i++){
		if(buf[i] != 'A'){
			printf("Error: read file all direct blocks incorrectly!\n");
			return FAIL;
		}
	}

	inode_free(inum);
	return PASS;
}

int test_inode_read_file_double_indirect_blo(){
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}
	//printf("111\n");
	int buf_size = (12 + 1024 + 250) * 4096;
	char buf[buf_size];
	for(int i = 0; i < buf_size; i++){
		buf[i] = 'A';
	}
	//printf("222\n");
	if(write_file(inum, buf, buf_size, 0) == -1){
		printf("Error: write file failed!\n");
		return FAIL;
	}
	//printf("333\n");
	int readsize = read_file(inum, buf, buf_size, 0);
	if( readsize != buf_size){
		printf("Error: read file readsize incorrect!\n");
		return FAIL;
	}
	for(int i = 0; i < buf_size; i++){
		if(buf[i] != 'A'){
			printf("Error: read file all direct blocks incorrectly!\n");
			return FAIL;
		}
	}

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
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}
	// Test using one single indir block
	int dir_size = 4096 * 12;
	char dir_buf[dir_size];
	for(int i = 0; i < dir_size; i++){
		dir_buf[i] = 'A';
	}
	if(write_file(inum, dir_buf, dir_size, 0) == -1){
		printf("Error: inode write file single indirect blo failed!\n");
		return FAIL;
	}
	char indir_buf[100];
	for(int i = 0; i < 100; i++){
		indir_buf[i] = 'B';
	}
	if(write_file(inum, indir_buf, 100, dir_size) == -1){
		printf("Error: inode write file single indirect blo failed!\n");
		return FAIL;
	}

	inode* target = find_inode_by_inum(inum);
	unsigned int indbid = target->single_ind;
	if(indbid == 0){
		printf("Error: write file single indirect block indbid not allocated!\n");
		return FAIL;
	}
	char indirblo[4096];
	unsigned int indirblobid[1024];
	db_read(indirblo, indbid);
	memcpy(indirblobid, indirblo, 4096);
	unsigned int firstindbid = indirblobid[0];
	if(firstindbid == 0){
		printf("Error: write file single indirect block firstindbid not allocated!\n");
		return FAIL;
	}
	char firstindblodata[4096];
	db_read(firstindblodata, firstindbid);
	//printf("indirblodata = %s\n", firstindblodata);
	for(int i = 0; i < 100; i++){
		if(firstindblodata[i] != 'B'){
			//printf("error at index %d\n", i);
			printf("Error: write file single indirect block file write incorrectly\n");
			return FAIL;
		}
	}
	free(target);

	// Test using full single indir blocks
	char full_indir_buf[4096];
	for(int i = 0; i < 4096; i++){
		full_indir_buf[i] = 'C';
	}
	int offset = 0;
	for(int i = 0; i < 12 + 1024; i++){
		write_file(inum, full_indir_buf, 4096, offset);
		offset += 4096;
	}
	inode* target2 = find_inode_by_inum(inum);
	indbid = target2->single_ind;
	if(indbid == 0){
		printf("Error: write file single indirect block indbid not allocated!\n");
		return FAIL;
	}
	db_read(indirblo, indbid);
	memcpy(indirblobid, indirblo, 4096);
	for(int i = 0; i < 1024; i++){
		if(indirblobid[i] == 0){
			printf("Error: write file single indirect block full firstindbid not allocated!\n");
			return FAIL;
		}
		db_read(firstindblodata, indirblobid[i]);
		for(int i = 0; i < 4096; i++){
			if(firstindblodata[i] != 'C'){
				printf("Error: write file single indirect block full file write incorrectly\n");
				return FAIL;
			}
		}
	}
	
	free(target2);
	inode_free(inum);
	return PASS;
}

int test_inode_write_file_double_indirect_blo(){
	int inum = inode_allocate();
	if(inum == -1){
		printf("Error: inode allocate failed!\n");
		return FAIL;
	}
	int buf_size = (12 + 1024 + 250) * 4096;
	char dbuf[buf_size];
	for(int i = 0; i < buf_size; i++){
		dbuf[i] = 'A';
	}
	if(write_file(inum, dbuf, buf_size, 0) == -1){
		printf("Error: write file single double indirect block failed!\n");
		return FAIL;
	}

	inode* target = find_inode_by_inum(inum);
	unsigned int dindbid = target->double_ind;
	//printf("dindbid = %u\n", dindbid);
	char dindbiddata[4096];
	db_read(dindbiddata,dindbid);
	//printf("dindbid data = %s\n", dindbiddata);
	unsigned int dindbidbids[1024];
	memcpy(dindbidbids, dindbiddata, 4096);
	//printf("dindbid bids0 = %u\n", dindbidbids[0]);
	//printf("dindbid bids1 = %u\n", dindbidbids[1]);

	unsigned int sindbid = dindbidbids[0];
	//printf("sindbid = %u\n", dindbidbids[0]);
	char sindbiddata[4096];
	unsigned int sindbidbids[1024];
	db_read(sindbiddata, sindbid);
	//printf("sindbid data = %s\n", sindbiddata);
	memcpy(sindbidbids, sindbiddata, 4096);
	//printf("sindbid bids0 = %u\n", sindbidbids[0]);
	//printf("sindbid bids1 = %u\n", sindbidbids[1]);


	char block[4096];
	for(int i = 0; i < 100; i++){
		//printf("sindbidbids %dth = %u\n", i, sindbidbids[i]);
		db_read(block, sindbidbids[i]);
		//printf("%dth block = %s\n",i,block);
		for(int z = 0; z < 4096; z++){
			if(block[z] != 'A'){
				printf("Error: write file double indirect block full file write incorrectly\n");
				return FAIL;
				
			}
		}
	}

	free(target);
	inode_free(inum);
	return PASS;
}


/*************************************super block test helpers***************************************/

int test_sb_init(){
	char super[4096];
	sb s;
	db_read(super,0);
	memcpy(&s, super, sizeof(sb));
	if(s.NUM_BLOCK != 262144 || s.filesize != 1073741824 || s.DIR_ID_NUM != 12){
		printf("Error: sb_init incorrectly\n");
		return FAIL;
	}
	return PASS;
}

int test_sb_read(){
	sb s;
	if(sb_read(&s) == -1){
		printf("Error: sb_read failed\n");
		return FAIL;
	}
	if(s.NUM_BLOCK != 262144 || s.filesize != 1073741824 || s.DIR_ID_NUM != 12){
		printf("Error: sb_init incorrectly\n");
		return FAIL;
	}
	return PASS;
}

int test_sb_write(){
	sb s;
	if(sb_read(&s) == -1){
		printf("Error: sb_read failed\n");
		return FAIL;
	}
	s.ROOT_INUM = 1;
	if(sb_write(&s) == -1){
		printf("Error: sb_write failed\n");
		return FAIL;
	}
	if(sb_read(&s) == -1){
		printf("Error: sb_read failed\n");
		return FAIL;
	}
	if(s.ROOT_INUM != 1){
		printf("Error: sb_write incorrectly\n");
		return FAIL;
	}
	s.ROOT_INUM = 0;
	if(sb_write(&s) == -1){
		printf("Error: sb_write failed\n");
		return FAIL;
	}
	return PASS;
}


/*************************************data block test helpers***************************************/

int test_db_multiple_allocate_free(){
	// First time allocate
	unsigned int bid;
	sb mysb;
	for(int i = 0; i < 262013; i++){
		bid = db_allocate();
		if(bid < 0 || bid > 262144){
			printf("Error: db_allocate failed at %dth block!\n", i);
			return FAIL;
		}
	}
	if(sb_read(&mysb) == -1){
		printf("Error: sb_read failed!\n");
		return FAIL;
	}
	if(mysb.NUM_FREE_BLOCK != 0){
		printf("Error: num free block incorrect!\n");
		return FAIL;
	}

	// First time free
	for(bid = 131; bid < 262144; bid++){
		db_free(bid);
	}
	if(sb_read(&mysb) == -1){
		printf("Error: sb_read failed!\n");
		return FAIL;
	}
	if(mysb.NUM_FREE_BLOCK != 262013){
		printf("Error: num free block incorrect!\n");
		return FAIL;
	}

	// Second time allocate
	for(int i = 0; i < 262013; i++){
		bid = db_allocate();
		if(bid < 0 || bid > 262144){
			printf("Error: db_allocate failed at %dth block!\n", i);
			return FAIL;
		}
	}
	if(sb_read(&mysb) == -1){
		printf("Error: sb_read failed!\n");
		return FAIL;
	}
	if(mysb.NUM_FREE_BLOCK != 0){
		printf("Error: num free block incorrect!\n");
		return FAIL;
	}

	// Second time free
	for(bid = 131; bid < 262144; bid++){
		db_free(bid);
	}
	if(sb_read(&mysb) == -1){
		printf("Error: sb_read failed!\n");
		return FAIL;
	}
	if(mysb.NUM_FREE_BLOCK != 262013){
		printf("Error: num free block incorrect!\n");
		return FAIL;
	}


	return PASS;
}

int test_db_allocate(){
	unsigned int bid;
	for(int i = 0; i < 262013; i++){
		bid = db_allocate();
		if(bid < 0 || bid > 262144){
			printf("Error: db_allocate failed at %dth block!\n", i);
			return FAIL;
		}
	}
	sb mysb;
	if(sb_read(&mysb) == -1){
		printf("Error: sb_read failed!\n");
		return FAIL;
	}
	if(mysb.NUM_FREE_BLOCK != 0){
		printf("Error: num free block incorrect!\n");
		return FAIL;
	}
	return PASS;
}

int test_db_free(){
	for(unsigned int bid = 131; bid < 262144; bid++){
		db_free(bid);
	}
	sb mysb;
	if(sb_read(&mysb) == -1){
		printf("Error: sb_read failed!\n");
		return FAIL;
	}
	if(mysb.NUM_FREE_BLOCK != 262013){
		printf("Error: num free block incorrect!\n");
		return FAIL;
	}

	return PASS;
}

int test_db_read(){
	return PASS;
}

int test_db_write(){
	return PASS;
}


/*************************************disk buffer cache test helpers***************************************/

int test_list_init(){
	List_Node* i;
	unsigned int acc;
	for(i = list_head, acc = 0; acc < BUFFER_NUM; i = i->next, acc++){
		if(i->buffer_id >= BUFFER_NUM || i->buffer_id < 0 ||i->in_hash != 0 || i->dirty != 0 || i->dirty != 0){
			// printf("acc = %d\n", acc);
			// printf("buffer_id = %d\n", i->buffer_id);
			printf("Error: test_list_init failed!\n");
			return FAIL;
		}
	}
	return PASS;
}

int test_list_free(){
	// list_free();
	// if(list_head != NULL || list_tail != NULL){
	// 	//printf("list head id = %u\n", list_head->buffer_id);
	// 	//printf("list tail id = %u\n", list_tail->buffer_id);
	// 	printf("Error: test_list_free failed!\n");
	// 	list_init();
	// 	return FAIL;
	// }
	// list_init();
	return PASS;
}

int test_list_add(){
	unsigned int bid = 0;
	char buffer[4096];

	list_add(bid, buffer, 0);
	if(list_head->block_id != bid){
		printf("Error: test_list_add failed!\n");
		return FAIL;
	}

	for(bid = 1; bid < 1001; bid++){
		list_add(bid, buffer, 0);
		if(list_head->block_id != bid){
			printf("Error: test_list_add failed!\n");
			return FAIL;
		}
	}
	return PASS;
}

int test_list_prioritize(){
	unsigned int bid;
	char buffer[4096];
	// prioritize first
	list_add(0, buffer, 0);
	bid = list_head->block_id;
	list_prioritize(list_head);
	if(list_head->block_id != bid){
		printf("Error: test_list_prioritize first element failed!\n");
		return FAIL;
	}

	for(bid = 1; bid < 1000; bid++){
		list_add(bid, buffer, 0);
	}

	// prioritize last
	bid = list_tail->block_id;
	list_prioritize(list_tail);
	if(list_head->block_id != bid){
		printf("Error: test_list_prioritize last element failed!\n");
		return FAIL;
	}

	// prioritize middle
	bid = list_head->next->next->next->next->next->block_id;
	list_prioritize(list_head->next->next->next->next->next);
	if(list_head->block_id != bid){
		printf("Error: test_list_prioritize middle element failed!\n");
		return FAIL;
	}

	return PASS;
}

int test_hash_init(){
	hash_free();
	hash_init();
	for(int i=0; i<BUFFER_NUM; i++){
		if(hash_table->list[i] != NULL){
			printf("Error: test_hash_init failed!\n");
			return FAIL;
		}
	}
	return PASS;
}

int test_hash_free(){
	// Hash_Node** l = hash_table->list;
	// hash_free();
	// if(hash_table != NULL){
	// 	printf("Error: test_hash_free hash_table failed!\n");
	// 	hash_init();
	// 	return FAIL;
	// }
	// if(l != NULL){
	// 	printf("Error: test_hash_free list failed!\n");
	// 	hash_init();
	// 	return FAIL;
	// }
	// hash_init();
	return PASS;
}

int test_hash_func(){
	int hash1 = hash_func(1);
	int hash2 = hash_func(1001);
	if(hash1 != hash2 || hash1 != 1 % BUFFER_NUM || hash2 != 1001 % BUFFER_NUM){
		printf("Error: test_hash_func failed!\n");
		return FAIL;
	}
	return PASS;
}

int test_hash_insert(){
	List_Node n1, n2, n3;
	n1.block_id = 1;
	n2.block_id = 2;
	n3.block_id = 1001;
	hash_insert(1,1, &n1);
	if(hash_table->list[1]->block_id != 1){
		printf("Error: test_hash_insert n1 failed!\n");
		return FAIL;
	}
	hash_insert(2,2, &n2);
	if(hash_table->list[2]->block_id != 2){
		printf("Error: test_hash_insert n2 failed!\n");
		return FAIL;
	}
	hash_insert(1001,3, &n3);
	if(hash_table->list[1]->block_id != 1001){
		printf("Error: test_hash_insert n3 failed!\n");
		return FAIL;
	}
	return PASS;
}

int test_hash_delete(){
	hash_delete(2);
	if(hash_table->list[2] != NULL){
		printf("Error: test_hash_delete n2 failed!\n");
		return FAIL;
	}
	hash_delete(1001);
	if(hash_table->list[1]->block_id != 1){
		printf("Error: test_hash_delete n3 failed!\n");
		return FAIL;
	}
	hash_delete(1);
	if(hash_table->list[1] != NULL){
		printf("Error: test_hash_delete n1 failed!\n");
		return FAIL;
	}
	return PASS;
}

int test_allocate_cache(){
	deallocate_cache();
	allocate_cache();
	if(buffer_0 == NULL){
		printf("Error: test_allocate_cache failed!\n");
		return FAIL;
	}
	return PASS;
}

int test_deallocate_cache(){
	// deallocate_cache();
	// if(buffer_0 != NULL){
	// 	printf("Error: test_deallocate_cache failed!\n");
	// 	allocate_cache();
	// 	return FAIL;
	// }
	// allocate_cache();
	return PASS;
}

int test_cache_to_disk(){
	char in[4096];
	char out[4096];
	for(int i = 0; i < 4096; i++){
		in[i] = 'A';
	}
	write_to_cache(in,0);
	cache_to_disk(0, 250);
	disk_read(out, 250);
	for(int i = 0; i < 4096; i++){
		if(out[i] != 'A'){
			printf("Error: test_cache_to_disk failed!\n");
			return FAIL;
		}
	}
	return PASS;
}

int test_write_to_cache(){
	char in[4096];
	for(int i = 0; i < 4096; i++){
		in[i] = 'A';
	}
	write_to_cache(in,0);
	for(int i = 0; i < 4096; i++){
		if(((char*)buffer_0)[i] != 'A'){
			printf("Error: test_write_to_cache failed!\n");
			return FAIL;
		}
	}
	return PASS;
}

int test_read_from_cache(){
	char out[4096];
	read_from_cache(out,0);
	for(int i = 0; i < 4096; i++){
		if(out[i] != 'A'){
			printf("Error: test_read_from_cache failed!\n");
			return FAIL;
		}
	}
	return PASS;
}

int test_sync(){
	char buffer[4096];
	buffer[0] = 'D';
	char out[4096];
	list_add(251,buffer, 1);
	sync();
	disk_read(out,251);
	if(out[0] != 'D'){
		printf("Error: test_sync failed!\n");
		return FAIL;
	}
	return PASS;
}



/***************************************test main********************************************/
int main(){


	if(allocate_disk("./disk") == -1){
		printf("allocation disk failed in test!\n");
		return -1;
	}
	allocate_cache();
    list_init();
    hash_init();
   

	/*---disk buffer cache tests---*/
	printf("-------------DISK BUFFER CACHE Test!-------------\n");

	printf("Test 1: test_list_init!\n");
	if(!test_list_init()){
		printf("	PASS: test_list_init!\n");
	}else{
		printf("	FAIL: test_list_init!\n");
	}

	printf("Test 2: test_list_free!\n");
	if(!test_list_free()){
		printf("	PASS: test_list_free!\n");
	}else{
		printf("	FAIL: test_list_free!\n");
	}

	printf("Test 3: test_list_add!\n");
	if(!test_list_add()){
		printf("	PASS: test_list_add!\n");
	}else{
		printf("	FAIL: test_list_add!\n");
	}

	printf("Test 4: test_list_prioritize!\n");
	if(!test_list_prioritize()){
		printf("	PASS: test_list_prioritize!\n");
	}else{
		printf("	FAIL: test_list_prioritize!\n");
	}

	printf("Test 5: test_hash_init!\n");
	if(!test_hash_init()){
		printf("	PASS: test_hash_init!\n");
	}else{
		printf("	FAIL: test_hash_init!\n");
	}

	printf("Test 6: test_hash_free!\n");
	if(!test_hash_free()){
		printf("	PASS: test_hash_free!\n");
	}else{
		printf("	FAIL: test_hash_free!\n");
	}

	printf("Test 7: test_hash_func!\n");
	if(!test_hash_func()){
		printf("	PASS: test_hash_func!\n");
	}else{
		printf("	FAIL: test_hash_func!\n");
	}

	printf("Test 8: test_hash_insert!\n");
	if(!test_hash_insert()){
		printf("	PASS: test_hash_insert!\n");
	}else{
		printf("	FAIL: test_hash_insert!\n");
	}

	printf("Test 9: test_hash_delete!\n");
	if(!test_hash_delete()){
		printf("	PASS: test_hash_delete!\n");
	}else{
		printf("	FAIL: test_hash_delete!\n");
	}

	printf("Test 10: test_allocate_cache!\n");
	if(!test_allocate_cache()){
		printf("	PASS: test_allocate_cache!\n");
	}else{
		printf("	FAIL: test_allocate_cache!\n");
	}

	printf("Test 11: test_deallocate_cache!\n");
	if(!test_deallocate_cache()){
		printf("	PASS: test_deallocate_cache!\n");
	}else{
		printf("	FAIL: test_deallocate_cache!\n");
	}

	printf("Test 12: test_cache_to_disk!\n");
	if(!test_cache_to_disk()){
		printf("	PASS: test_cache_to_disk!\n");
	}else{
		printf("	FAIL: test_cache_to_disk!\n");
	}

	printf("Test 13: test_write_to_cache!\n");
	if(!test_write_to_cache()){
		printf("	PASS: test_write_to_cache!\n");
	}else{
		printf("	FAIL: test_write_to_cache!\n");
	}

	printf("Test 14: test_read_from_cache!\n");
	if(!test_read_from_cache()){
		printf("	PASS: test_read_from_cache!\n");
	}else{
		printf("	FAIL: test_read_from_cache!\n");
	}

	printf("Test 15: test_sync!\n");
	if(!test_sync()){
		printf("	PASS: test_sync!\n");
	}else{
		printf("	FAIL: test_sync!\n");
	}

	
	// recreate disk buffer
	free_disk();
    deallocate_cache();
    list_free();
    hash_free();

	if(allocate_disk("./disk") == -1){
		printf("allocation disk failed in test \n");
		return -1;
	}
	allocate_cache();
    list_init();
    hash_init();


	// create sb and db
	sb_init();        
	if(db_init() == -1){
		printf("dn init error \n");
		return -1;
	}

	/*---sb tests---*/

	printf("-------------Running SB Tests!-------------\n");
	
	printf("Test 1: test_sb_init!\n");
	if(!test_sb_init()){
		printf("	PASS: test_sb_init!\n");
	}else{
		printf("	FAIL: test_sb_init\n");
	}

	printf("Test 2: test_sb_read!\n");
	if(!test_sb_read()){
		printf("	PASS: test_sb_read!\n");
	}else{
		printf("	FAIL: test_sb_read\n");
	}

	printf("Test 3: test_sb_write!\n");
	if(!test_sb_write()){
		printf("	PASS: test_sb_write!\n");
	}else{
		printf("	FAIL: test_sb_write\n");
	}


	// /*---db tests---*/

	printf("-------------Running DB Tests!-------------\n");

		printf("Test 1: test_db_multiple_allocate_free!\n");
	if(!test_db_multiple_allocate_free()){
		printf("	PASS: test_db_multiple_allocate_free!\n");
	}else{
		printf("	FAIL: test_db_multiple_allocate_free\n");
	}

		printf("Test 2: test_db_allocate!\n");
	if(!test_db_allocate()){
		printf("	PASS: test_db_allocate!\n");
	}else{
		printf("	FAIL: test_db_allocate\n");
	}

		printf("Test 3: test_db_free!\n");
	if(!test_db_free()){
		printf("	PASS: test_db_free!\n");
	}else{
		printf("	FAIL: test_db_free\n");
	}

		printf("Test 4: test_db_read!\n");
	if(!test_db_read()){
		printf("	PASS: test_db_read!\n");
	}else{
		printf("	FAIL: test_db_read\n");
	}

		printf("Test 5: test_db_write!\n");
	if(!test_db_write()){
		printf("	PASS: test_db_write!\n");
	}else{
		printf("	FAIL: test_db_write\n");
	}
	


	/*---inode tests---*/
	printf("-------------Running INODE Tests!-------------\n");
	
	printf("Test 1: test_bitmap_init!\n");
	if(!test_bitmap_init()){
		printf("	PASS: test_bitmap_init!\n");
	}else{
		printf("	FAIL: test_bitmap_init\n");
	}

	printf("Test 2: test_inode_list_init!\n");
	if(!test_inode_list_init()){
		printf("	PASS: test_inode_list_init!\n");
	}else{
		printf("	FAIL: test_inode_list_init!\n");
	}

	printf("Test 3: test_inode_allocate!\n");
	if(!test_inode_allocate()){
		printf("	PASS: test_inode_allocate!\n");
	}else{
		printf("	FAIL: test_inode_allocate!\n");
	}

	printf("Test 4: test_inode_free!\n");
	if(!test_inode_free()){
		printf("	PASS: test_inode_free!\n");
	}else{
		printf("	FAIL: test_inode_free!\n");
	}

	printf("Test 5: test_inode_mode_read_write!\n");
	if(!test_inode_mode_read_write()){
		printf("	PASS: test_inode_mode_read_write!\n");
	}else{
		printf("	FAIL: test_inode_mode_read_write!\n");
	}

	printf("Test 6: test_inode_link_read_reduce!\n");
	if(!test_inode_link_read_reduce()){
		printf("	PASS: test_inode_link_read_reduce!\n");
	}else{
		printf("	FAIL: test_inode_link_read_reduce!\n");
	}

	printf("Test 7: test_inode_read_size!\n");
	if(!test_inode_read_size()){
		printf("	PASS: test_inode_read_size!\n");
	}else{
		printf("	FAIL: test_inode_read_size!\n");
	}

	printf("Test 8: test_inode_rootnum!\n");
	if(!test_inode_rootnum()){
		printf("	PASS: test_inode_rootnum!\n");
	}else{
		printf("	FAIL: test_inode_rootnum!\n");
	}

	printf("Test 9: test_inode_read_file_direct_blo!\n");
	if(!test_inode_read_file_direct_blo()){
		printf("	PASS: test_inode_read_file_direct_blo!\n");
	}else{
		printf("	FAIL: test_inode_read_file_direct_blo!\n");
	}

	printf("Test 9b: test_inode_read_file_single_indirect_blo!\n");
	if(!test_inode_read_file_single_indirect_blo()){
		printf("	PASS: test_inode_read_file_single_indirect_blo!\n");
	}else{
		printf("	FAIL: test_inode_read_file_single_indirect_blo!\n");
	}

	printf("Test 9c: test_inode_read_file_double_indirect_blo!\n");
	if(!test_inode_read_file_double_indirect_blo()){
		printf("	PASS: test_inode_read_file_double_indirect_blo!\n");
	}else{
		printf("	FAIL: test_inode_read_file_double_indirect_blo!\n");
	}

	printf("Test 10a: test_inode_write_file_direct_blo!\n");
	if(!test_inode_write_file_direct_blo()){
		printf("	PASS: test_inode_write_file_direct_blo!\n");
	}else{
		printf("	FAIL: test_inode_write_file_direct_blo!\n");
	}

	printf("Test 10b: test_inode_write_file_single_indirect_blo!\n");
	if(!test_inode_write_file_single_indirect_blo()){
		printf("	PASS: test_inode_write_file_single_indirect_blo!\n");
	}else{
		printf("	FAIL: test_inode_write_file_single_indirect_blo!\n");
	}

	printf("Test 10c: test_inode_write_file_double_indirect_blo!\n");
	if(!test_inode_write_file_double_indirect_blo()){
		printf("	PASS: test_inode_write_file_double_indirect_blo!\n");
	}else{
		printf("	FAIL: test_inode_write_file_double_indirect_blo!\n");
	}
	
	

	free_disk();
    deallocate_cache();
    list_free();
    hash_free();

	return 0;
}
