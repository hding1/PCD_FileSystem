#define INODE_MAX_NUM 50000
#define DIRECT_BLKS_NUM 10

struct inode{
     //owner info
     int permission;
     int file_name;
     int file_type;
     int path;
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
extern inode ilist[INODE_MAX_NUM];

inode_list_init(); //initlaize inode list and write all inodes to the disk
int inode_read(inode* node); //read inode from the disk with index 
int inode_write(inode* node); // write inode to the disk
int inode_free(int index); // free inode from inode list at index i
inode* inode_allocate(); // allocate an inode