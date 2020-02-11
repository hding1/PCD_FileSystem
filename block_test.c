//author: Fuheng
#include "sb.h"
#include "db.h"
#include <assert.h>
#include <stdio.h>
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
	return 0;
}
