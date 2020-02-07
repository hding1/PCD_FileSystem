// Responsible author(s): Dennis

#include <inode.h>


unsigned int inode_bitmap_init(){

    // Create a bitmap of size NUM_INODE, set all inodes to be free
    bool bitmap[NUM_INODE];
    for(int i = 0; i < NUM_INODE; i++){
        bitmap[i] = false;
    }

    // Allocate the 2nd block (block id = 1) for inode bitmap
    memcpy((db*) (add_0 + 1 * 4096), bitmap, sizeof(bitmap));

    // Return the starting block id
    return 1;
}

unsigned int find_free_inode(){
    // STUB
    return 0;
}


unsigned int inode_list_init(){

     // Create a default inode struct
    inode node;
    node->mode = 0;
    node->UID  = 0;
    node->GID  = 0;
    node->size = 0;
    node->last_accessed = time(NULL);
    node->last_modified = time(NULL);
    for(int i = 0; i < DIRECT_BLKS_NUM; i++){
        node->direct_blo[i] = -1;
    }
    node->single_ind = -1;
    node->double_ind = -1;
    node->triple_ind = -1;

    // Allocate the 3rd to 130th block (block id =2, 129) for inode list
    for(unsigned int bid = 2; bid < 130; bid++){
        char* b_add = (char*) ( ((db*) add_0) + bid );
        for(int i = 0; i < 32; i++){
            memcpy(b_add + i * INODE_SIZE, &node, INODE_SIZE);
        }
    }
    // Return the starting block id
    return 2;
}

inode* find_inode_by_inum(unsigned int inum){
    //STUB
    return NULL;
}


unsigned int inode_allocate(){

    // Find a opening in the bitmap
    unsigned int inum = find_free_inode();

    // Set inode attributes if necessary
    // STUB

    // Return the inum of the inode created
    return inum; 
}

unsigned int inode_free(unsigned int inum){
    
    // Find the inode by inum

    // reduce ref count by 1; If ref count become less than 1 free the inode in the bitmap
    
    // Return the inum of the inode freed
    return inum;

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