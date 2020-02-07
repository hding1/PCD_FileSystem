// Responsible author(s): Dennis

#include <inode.h>


int inode_bitmap_init(){

    // Create a bitmap of size NUM_INODE, set all inodes to be free
    bool bitmap[NUM_INODE];
    for(int i = 0; i < NUM_INODE; i++){
        bitmap[i] = false;
    }

    // Wirte the bitmap to disk
    db_write(bitmap, BITMAP_BID);

    // Return 0 on success
    return 0;
}

unsigned int find_free_inode(){
    // Bring the bitmap from disk to memory
    bool bitmap[NUM_INODE];
    db_read(bitmap, BITMAP_BID);
    for(int i = 0; i < NUM_INODE; i++){
        if(bitmap[i] == false){
            bitmap[i] = true;
            return i;   // Return the inum of the free inode
        }
    }
    return NUM_INODE; // If unable to find a free inode
}


int inode_list_init(){

     // Create a default inode struct
    inode node;
    node->mode = 0;
    node->UID  = 0;
    node->GID  = 0;
    node->size = 0;
    node->last_accessed = 0;
    node->last_modified = 0;
    for(int i = 0; i < DIRECT_BLKS_NUM; i++){
        node->direct_blo[i] = 0;
    }
    node->single_ind = 0;
    node->double_ind = 0;
    node->triple_ind = 0;

    // Allocate the 3rd to 130th block (block id =2, 129) for inode list
    char block[4096];
    for(unsigned int bid = 2; bid < 130; bid++){
        memset(block, 0, 4096);
        for(int i = 0; i < 32; i++){
            memcpy(block + i * INODE_SIZE, &node, sizeof(node));
        }
        db_write(block, bid);
    }
    // Return 0 on success
    return 0;
}

inode* find_inode_by_inum(unsigned int inum){
    unsigned int bid = ILIST_BID + inum / 32;
    unsigned offset = inum % 32;
    inode* ptr = (inode*)((db*)add_0 + bid) + offset;
    return ptr;
}

unsigned int inode_allocate(){

    // Find a opening in the bitmap
    unsigned int inum = find_free_inode();

    // Set inode attributes if necessary
    node->last_accessed = time(NULL);
    node->last_modified = time(NULL);

    // Return the inum of the inode created
    return inum; 
}

unsigned int inode_free(unsigned int inum){
    
    // Mark the inode free in bitmap
    bool bitmap[NUM_INODE];
    db_read(bitmap, BITMAP_BID);
    bitmap[inum] = 0;
    db_write(bitmap, BITMAP_BID);

    // Return the inum of the inode freed
    return inum;
}

int inode_read(inode* out, unsigned int inum){

    // Find the inode by inum
    inode* ptr = find_inode_by_inum(inum);

    // load the inode (128 bytes) to buffer
    
    return 0; // If success
}

int inode_write(inode* in, unsigned int inum){

    // Find the inode by inum
    inode* ptr = find_inode_by_inum(inum);

    // load the buffer (128 bytes) to inode

   return 0; // If success
}
