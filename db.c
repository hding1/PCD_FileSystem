// Responsible author(s): Fuheng Zhao
#include <string.h>
#include <stdio.h>
#include <db.h>

int allocate(db* Disk_Buffer){
	Disk_Buffer = (struct db*)malloc(sizeof(struct db));
	Disk_Buffer->Block = malloc(4096*sizeof(char));
return 1;
}
int free(db* Disk_Buffer){
//memset(Disk_Buff.content, 0, 4096);
	free(Disk_Buffer->Block);
	return 1;
}
int read(db* out, db* Disk_Buffer){
	disk_read(out,Disk_Buffer);
	return 1;
}
int write(db* in, db* Disk_Buffer){
	disk_write(in, Disk_Buffer);
	return 1;
}

void disk_read(db* out, db* Disk_Buffer){
	memcpy(out->Block, Disk_Buffer->Block,4096);
}
void disk_write(db* in, db* Disk_Buffer){
	memcpy(Disk_Buffer->Block, in->Block,4096);
}


