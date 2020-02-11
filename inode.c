// Responsible author(s): Dennis

#include "inode.h"

int inode_bitmap_init(){

    // Create a bitmap of size NUM_INODE, set all inodes to be free
    unsigned short bitmap[NUM_INODE];
    for(int i = 0; i < NUM_INODE; i++){
        bitmap[i] = 0;
    }

    // Wirte the bitmap to disk
    db_write(bitmap, BITMAP_BID);

    return 0;     // Return 0 on success, -1 on error
}

int find_free_inode(){

    // Bring the bitmap from disk to memory
    unsigned short bitmap[NUM_INODE];

    int status = db_read(bitmap, BITMAP_BID);

    for(int i = 0; i < NUM_INODE; i++){
        if(bitmap[i] == 0){
            bitmap[i] = 1;
            return i;   // Return the inum of the free inode
        }
    }
    return -1; // If unable to find a free inode
}


int inode_list_init(){

    // Create a default inode struct
    inode node;
    node.mode = 0;
    node.UID  = 0;
    node.GID  = 0;
    node.size = 0;
    node.last_accessed = 0;
    node.last_modified = 0;
    for(int i = 0; i < DIRECT_BLKS_NUM; i++){
        node.direct_blo[i] = 0;
    }
    node.single_ind = 0;
    node.double_ind = 0;
    node.triple_ind = 0;
    node.link_count = 0;

    // Allocate the 3rd to 130th block (block id =2, 129) for inode list
    char block[4096];
    for(unsigned int bid = ILIST_BID; bid < 130; bid++){
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

    if(inum < ROOT_INUM || inum > NUM_INODE - 1){
        return NULL;
    }

    // Read the block
    char block[4096];
    unsigned int bid = ILIST_BID + inum / 32;
    db_read(block, bid);
    // Read the inode
    inode* node = (inode*) malloc(sizeof(inode));
    unsigned offset = inum % 32;
    char* ptr = (char*) block + offset * INODE_SIZE;    // Find the inode in the block
    memcpy(node, ptr, sizeof(inode));

    return node;
}

void write_inode_to_disk(unsigned int inum, inode* target_node){
    char block[4096];
    unsigned int bid = ILIST_BID + inum / 32;
    db_read(block, bid);
    unsigned offset = inum % 32;
    char* node_ptr = (char*) block + offset * INODE_SIZE;
    memcpy(node_ptr, target_node, sizeof(inode));
    free(target_node);
    block_write(block, bid);
}


int inode_allocate(){

    // Find a opening in the bitmap
    unsigned int inum = find_free_inode();
    if(inum == -1) return -1;   // Error

    // Get the inode, set initial values
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -2;  // Error
    node->last_accessed = time(NULL);
    node->last_modified = time(NULL);

    // Write changes back to disk
    char block[4096];
    unsigned int bid = ILIST_BID + inum / 32;
    db_read(block, bid);
    unsigned offset = inum % 32;
    char* node_ptr = (char*) block + offset * INODE_SIZE;
    memcpy(node_ptr, target_node, sizeof(inode));
    free(target_node);
    block_write(block, bid);
    // Return the inum if success
    return inum; 
}

int inode_free(unsigned int inum){

    // Free data blocks used    
    inode* node = find_inode_by_inum(inum);
    // STUB

    // Mark the inode free in bitmap
    unsigned short bitmap[NUM_INODE];
    db_read(bitmap, BITMAP_BID);
    bitmap[inum] = 0;
    db_write(bitmap, BITMAP_BID);

    // Return 0 on success
    return 0;
}


int inode_read(unsigned int inum, mode_t* mode_out){

    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;   // Error
    *mode_out = target_node->mode;
    free(target_node);
    return 0; // If success
}

int inode_write(unsigned int inum, mode_t* mode_in){
    
    // Read the block containing the target inode
    char block[4096];
    unsigned int bid = ILIST_BID + inum / 32;
    db_read(block, bid);
    // Find the target inode position inside the block
    unsigned offset = inum % 32;
    char* node_ptr = (char*) block + offset * INODE_SIZE;
    // Modify mode of target inode
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1; // Error
    target_node->mode = *mode_in;
    // Write the modified inode into block
    memcpy(node_ptr, target_node, sizeof(inode));
    free(target_node);
    // Write back to disk
    db_write(block, bid);
    return 0; // If success
}


unsigned int get_root_inum(){
    return 0;
}









int read_file(unsigned int inum, char* buf, int size, int offset){
    // Read inode
    // Locate offset
    // Read disk to buf
}


int write_file(unsigned int inum, char* buf, int size, int offset){
    // Read inode
    // Locate offset
    // Write buffer to disk
}