// Responsible author(s): Dennis

#include <inode.h>

int inode_bitmap_init(){

    // Create a bitmap of size NUM_INODE, set all inodes to be free
    unsigned short bitmap[NUM_INODE];
    for(int i = 0; i < NUM_INODE; i++){
        bitmap[i] = 0;
    }

    // Wirte the bitmap to disk
    int status = db_write(bitmap, BITMAP_BID);

    return status;     // Return 0 on success, -1 on error
}

int find_free_inode(){

    // Bring the bitmap from disk to memory
    unsigned short bitmap[NUM_INODE];

    int status = db_read(bitmap, BITMAP_BID);
    if(!status) return -1; // error reading bitmap

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
    int status;
    for(unsigned int bid = 2; bid < 130; bid++){
        memset(block, 0, 4096);
        for(int i = 0; i < 32; i++){
            memcpy(block + i * INODE_SIZE, &node, sizeof(node));
        }
        status = db_write(block, bid);
        if(!status) return -1;  // error writing inode
    }
    // Return 0 on success
    return 0;
}


inode* find_inode_by_inum(unsigned int inum){
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


int inode_allocate(inode** out){

    // Find a opening in the bitmap
    unsigned int inum = find_free_inode();
    if(inum == -1) return -1;
    // Get the inode, set initial values
    inode_read(out, inum);
    (*out)->last_accessed = time(NULL);
    (*out)->last_modified = time(NULL);
    // Write changes back to disk
    inode_write(out, inum);

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


int inode_read(inode** out, unsigned int inum){

    // Read the block
    char block[4096];
    unsigned int bid = ILIST_BID + inum / 32;
    db_read(block, bid);
    // Read the inode
    inode* node = (inode*) malloc(sizeof(inode));
    unsigned offset = inum % 32;
    char* ptr = (char*) block + offset * INODE_SIZE;    // Find the inode in the block
    memcpy(node, ptr, sizeof(inode));
    // Set out
    *out = node;

    return 0; // If success
}

int inode_write(inode** in, unsigned int inum){
    
    // Read the block from disk
    char block[4096];
    unsigned int bid = ILIST_BID + inum / 32;
    db_read(block, bid);
    // Write the inode in the block
    unsigned offset = inum % 32;
    char* ptr = (char*) block + offset * INODE_SIZE;    
    memcpy(ptr, *in, sizeof(inode));
    // Write back to disk
    db_write(block, bid);

   return 0; // If success
}


unsigned int get_root_inum();
int read_file(inode** out, char** buf, int size, int offset);
int write_file(inode** in, char** buf, int size, int offset);