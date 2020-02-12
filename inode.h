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

#ifndef INODE_H_
#define INODE_H_

#define NUM_INODE 4096
#define DIR_ID_NUM 12
#define INDIR_ID_NUM 1024
#define D_INDIR_ID_NUM 1024*1024
#define T_INDIR_ID_NUM 1024*1024*1024
#define INODE_SIZE 128
#define ROOT_INUM 0
#define MAX_FILE_SIZE 

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
     unsigned long size;

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
int inode_list_init();

// Helper function
int find_free_inode();
inode* find_inode_by_inum(unsigned int inum);
int write_inode_to_disk(unsigned int inum, inode* target_node);
int free_indblo_by_bid(unsigned int bid);
int free_dindblo_by_bid(unsigned int bid);
int free_tindblo_by_bid(unsigned int bid);
unsigned int find_block_by_num(unsigned int inum, unsigned int num);  // return the bid of the nth block in this inode
int write_block_by_num(unsigned int inum, unsigned int num, char* block);
int add_block(unsigned int inum);

// Individual inode operations
int inode_allocate();
int inode_free(unsigned int inum);
int inode_read_mode(unsigned int inum, mode_t* mode_out);
int inode_write_mode(unsigned int inum, mode_t* mode_in);
int inode_read_size(unsigned int inum, unsigned long* size); 
int inode_read_link_count(unsigned int inum, unsigned int* count); 
int inode_reduce_link(unsigned int inum); 

unsigned int get_root_inum();
int read_file(unsigned int inum, char* buf, int size, int offset);
int write_file(unsigned int inum, char* buf, int size, int offset);

#endif




// TO DO
// 1. use sb instead hardcoded bid
// 2. figure out how to set UID GID
// 3. free block only free used blocks
// 4. safety checks
// 5. read file boundary


#endif //PCD_FILESYSTEM_INODE_H_
