// Responsible author(s): DZ

#include <stdlib.h>
#include <db.h>

#define DIRECT_BLKS_NUM 12

typedef struct{
     /* Mode: keeps information about two things, 
               1) permission information, 
               2) type of inode */
     

     /* Owner info: Access details like owner of the file, 
                    group of the file etc */
     int UID;
     int GID;

     /* Size: size of the file in terms of bytes */
     int size;

     // Time stampes
     int last_accessed;
     int last_modified;

     // Data blocks
     db* direct_blocks[DIRECT_BLKS_NUM];
     inode* single_ind;
     inode* double_ind;
     inode* triple_ind;   
}inode;

int inode_list_init();
int inode_read(inode* node);
int inode_write(inode* node);
