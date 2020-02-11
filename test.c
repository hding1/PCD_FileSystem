#include "sb.h"
#include "db.h"

int main(){
	
	allocate_disk();
	sb_init();
	db_init();

	return 0;
}
