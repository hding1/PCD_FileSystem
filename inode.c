// Responsible author(s): Dennis

#include <inode.h>


unsigned int inode_bitmap_init(){

    // Allocate the 2nd block and 3rd block (block id = 1, 2) for inode bitmap

    // Set all inode to free

    // Return the starting block id
    return 1;
}


unsigned int inode_list_init(){

    // Allocate the 4th to 131th block (block id =3, 130) for inode list

    // Return the starting block id
    return 3;
}


unsigned int inode_allocate(){

    // Find a opening in the bitmap

    // Create a default inode struct
    inode node;
    node->permission = 666;
    node->type = 0;
    node->UID = 0;
    node->GID = 0;
    node->size = 4096;
    node->last_accessed = time(NULL);
    node->last_modified = time(NULL);
    for(int i = 0; i < DIRECT_BLKS_NUM; i++){
        node->direct_blo[i] = NULL;
    }
    node->single_ind = NULL;
    node->double_ind = NULL;
    node->triple_ind = NULL;

    // Write the inode struct to the place indicated by bitmap in the disk 

    // Return the inum of the inode created
    return 0; 
}

unsigned int inode_free(unsigned int inum){
    
    // Free the inode in the bitmap

    // Return the inum of the inode freed
    return 0;

}

int inode_read(char* out_buffer, unsigned int inum){

    // Find the inode by inum

    // load the inode (128 bytes) to buffer
    
    return 0; // If success
}

int inode_write(char* in_buffer, unsigned int inum){

    // Find the inode by inum

    // load the buffer (128 bytes) to inode

   return 0; // If success
}