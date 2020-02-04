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

unsigned int inode_allocate();
unsigned int inode_free(unsigned int inum);
int inode_read(char* out_buffer, unsigned int inum);
int inode_write(char* in_buffer, unsigned int inum);

unsigned int inode_list_init();


// Layer 1.5 - File io by inode id
int get_root_inum(int* inum);
int allocate_file(int* inum, mode_t mode);
int chmod(int* inum, mode_t mode);
int chmod(int* inum, mode_t mode);
int free_file(int inum);
int read_file(int inum, char* buf, int size, int offset);
int write_file(int inum, char* buf, int size, int offset);