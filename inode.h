// Responsible author(s): Dennis

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <db.h>

#define DIRECT_BLKS_NUM 12

typedef struct inode{
     /* Mode: keeps information about two things, 
               1) permission information, 
               2) type of inode */
     mode_t mode;

     /* Owner info: Access details like owner of the file, 
                    group of the file etc */
     unsigned int UID;
     unsigned int GID;

     /* Size: size of the file in terms of bytes */
     // Default file size is set to 4096 bytes (one data block)
     unsigned int size;

     // Time stampes
     time_t last_accessed;
     time_t last_modified;

     // Data blocks
     unsigned int direct_blo[DIRECT_BLKS_NUM];
     unsigned int single_ind;
     unsigned int double_ind;
     unsigned int triple_ind;   
}inode;

int inode_allocate(struct inode* node);
int inode_free(struct inode* node);
int inode_read(struct inode* node, char* disk_buffer);
int inode_write(struct inode* node, char* disk_buffer);

int inode_list_init(struct inode* list);