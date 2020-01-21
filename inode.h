
#define DIRECT_BLKS_NUM 

struct inode{
     //owner info

     //size
     int size;
     //link
     int link;
     //time stamped
     int last_accessed;
     int last_modified;
     //data blocks
     int db[DIRECT_BLKS_NUM];
     int single_ind;
     int double_ind;
     int triple_ind;   
}

inode_list_init();
int inode_read(inode* node);
int inode_write(inode* node);