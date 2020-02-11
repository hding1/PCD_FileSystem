#ifndef PCD_FILESYSTEM_INODE_H_
#define PCD_FILESYSTEM_INODE_H_

// Responsible author(s): Dennis

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "db.h"
#include "fs.h"

#include "db.h"

#define NUM_INODE 4096
#define DIRECT_BLKS_NUM 12
#define INODE_SIZE 128
#define ROOT_INUM 0

#define BITMAP_BID 1
#define ILIST_BID 2

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
     // Default file size is 0
     unsigned int size;

     // Time stampes
     time_t last_accessed;
     time_t last_modified;

     // Data blocks
     unsigned int direct_blo[DIRECT_BLKS_NUM];
     unsigned int single_ind;
     unsigned int double_ind;
     unsigned int triple_ind;

     // Status
     unsigned int link_count;   
}inode;

// Allocate space for inode bitmap and inode list
int inode_bitmap_init();
int find_free_inode();
int inode_list_init();

// Helper function
inode* find_inode_by_inum(unsigned int inum);

// Individual inode operations
int inode_allocate();
int inode_free(unsigned int inum);
int inode_read_mode(unsigned int inum, mode_t* mode_out);
int inode_write_mode(unsigned int inum, mode_t* mode_in);

// Layer 1.5 - File io by inode id

// int allocate_file(int* inum, mode_t mode);
// int chmod(int* inum, mode_t mode);
// int chmod(int* inum, mode_t mode);
// int free_file(int inum);
unsigned int get_root_inum();
int read_file(unsigned int inum, char* buf, int size, int offset);
int write_file(unsigned int inum, char* buf, int size, int offset);





// TO DO
// 1. use sb instead hard coded bid
// 2. figure out how to set UID GID

#endif //PCD_FILESYSTEM_INODE_H_