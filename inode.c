// Responsible author(s): Dennis

#include "inode.h"


/*********************************Init functions************************************/
int inode_bitmap_init(){

    // Create a bitmap of size NUM_INODE, set all inodes to be free
    unsigned short bitmap[NUM_INODE];
    for(int i = 0; i < NUM_INODE; i++){
        bitmap[i] = 0;
    }

    // Wirte the bitmap to disk
    db_write(bitmap, BITMAP_BID);

    return 0; 
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
    for(int i = 0; i < DIR_ID_NUM; i++){
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

    return 0;
}


/**********************************Helper function****************************************/

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

int write_inode_to_disk(unsigned int inum, inode* target_node){
    
    if(inum < ROOT_INUM || inum > NUM_INODE - 1){
        return -1;
    }
    if(target_node == NULL) return -1;

    char block[4096];
    unsigned int bid = ILIST_BID + inum / 32;
    db_read(block, bid);
    unsigned offset = inum % 32;
    char* node_ptr = (char*) block + offset * INODE_SIZE;
    memcpy(node_ptr, target_node, sizeof(inode));
    free(target_node);
    block_write(block, bid);

    return 0;
}

int free_indblo_by_bid(unsigned int bid){
    char block[DB_SIZE];
    db_read(block, bid);
    unsigned int bids[INDIR_ID_NUM];
    memcpy(bids, block, DB_SIZE);
    for(int i = 0; i < DIR_ID_NUM; i++){
        db_free(bids[i]);
    }
    return 0;
}

int free_dindblo_by_bid(unsigned int bid){
    char ind[DB_SIZE];
    db_read(block, bid);
    unsigned int bids[INDIR_ID_NUM];
    memcpy(bids, block, DB_SIZE);
    for(int i = 0; i < INDIR_ID_NUM; i++){
        free_indblo_by_bid(bids[i]);
    }
}

int free_tindblo_by_bid(unsigned int bid){
    char ind[DB_SIZE];
    db_read(block, bid);
    unsigned int bids[INDIR_ID_NUM];
    memcpy(bids, block, DB_SIZE);
    for(int i = 0; i < INDIR_ID_NUM; i++){
        free_dindblo_by_bid(bids[i]);
    }
}

char* find_block_by_bid(unsigned int bid){
    char* buffer = (char*) malloc(DB_SIZE*sizeof(char));
    //STUB
    return block;
}


/******************************************Inode opertaions***********************************************/

int inode_allocate(){

    // Find a opening in the bitmap
    unsigned int inum = find_free_inode();
    if(inum == -1) return -1;   // Error

    // Get the inode, set initial values
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;  // Error
    node->last_accessed = time(NULL);
    node->last_modified = time(NULL);

    // Write changes back to disk
    int status = write_inode_to_disk(inum, target_node);
    if(!status) return -1;

    // Return the inum if success
    return inum; 
}

int inode_free(unsigned int inum){

    // Free data blocks used    
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;
    // Find how many blocks used
    int num_blo = target_node->size / DB_SIZE;
    int offset = target_node->size % DB_SIZE;
    if(offset != 0) num_blo++;
    // Free those blocks
    if(num_blo <= DIR_ID_NUM){
        for(int i = 0; i < num_blo; i++){
            block_free(direct_blo[i]);
        }
    }else if(num_blo <= INDIR_ID_NUM + DIR_ID_NUM){
        for(int i = 0; i < num_blo; i++){
            block_free(direct_blo[i]);
        }
        free_indblo_by_bid(single_ind);
    }else if(num_blo <= D_INDIR_ID_NUM + INDIR_ID_NUM + DIR_ID_NUM){
        for(int i = 0; i < num_blo; i++){
            block_free(direct_blo[i]);
        }
        free_indblo_by_bid(single_ind);
        free_dindblo_by_bid(double_ind);
    }else if(num_blo <= T_INDIR_ID_NUM + D_INDIR_ID_NUM + INDIR_ID_NUM + DIR_ID_NUM){
        for(int i = 0; i < num_blo; i++){
            block_free(direct_blo[i]);
        }
        free_indblo_by_bid(single_ind);
        free_dindblo_by_bid(double_ind);
        free_tindblo_by_bid(triple_ind);
    }else{
        return -1;
    }

    // Mark the inode free in bitmap
    unsigned short bitmap[NUM_INODE];
    db_read(bitmap, BITMAP_BID);
    bitmap[inum] = 0;
    db_write(bitmap, BITMAP_BID);

    return 0; // If success
}

int inode_read_mode(unsigned int inum, mode_t* mode_out){

    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;   // Error
    *mode_out = target_node->mode;
    free(target_node);

    return 0; // If success
}

int inode_write_mode(unsigned int inum, mode_t* mode_in){
    
    // Modify mode of target inode
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1; // Error
    target_node->mode = *mode_in;
    // Write the modified inode into block
    int status = write_inode_to_disk(inum, target_node);
    if(!status) return -1;

    return 0; // If success
}

int inode_read_size(unsigned int inum, unsigned long* size){

    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;   // Error
    *size = target_node->size;
    free(target_node);

    return 0; // If success
}

int inode_read_link_count(unsigned int inum, unsigned int* count){

    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;   // Error
    *count = target_node->link_count;
    free(target_node);

    return 0; // If success
}


/********************************************Layer1.5 functions***********************************************/

unsigned int get_root_inum(){
    return ROOT_INUM;
}

int read_file(unsigned int inum, char* buf, int size, int offset){

    // Read inode
    inode * myNode;
    myNode = find_inode_by_inum(inum);
    // Locate offset 
    unsigned int startBlock = offset/4096;
    unsigned int endBlock = (offset+size)/4096;
    unsigned int toRead = 4096-offset%4096;
    unsigned int indirBuf1[1024];
    unsigned int indirBuf2[1024];
    unsigned int indirBuf3[1024];
    // Read disk to buf
    while(startBlock<=endBlock){
     if(startBlock<=DIRECT_BLKS_NUM){
      //disk read (startblock, toRead)
     }else
     if(startBlock<=(DIRECT_BLKS_NUM+1024)){
      //assign indirBuf1
      //disk read (indirBuf1[startblock-12], toRead)
     }else
     if(startBlock<=(DIRECT_BLKS_NUM+1024*1024)){
      //assign indirBuf1
      //assign indirBuf2
      //disk read (indirBuf1[indirBuf2[(startblock-12)%1024]], toRead)
     }else
     if(startBlock<=(DIRECT_BLKS_NUM+1024*1024*1024)){
      //assign indirBuf1
      //assign indirBuf2
      //assign indirBuf3
      //same as above
     }else{
      //error
     }
     startBlock++;
     size-=toRead;
     if(size>=4096){
      toRead = 4096;
     }
     else{
      toRead = size;
     }
    }
}

int write_file(unsigned int inum, char* buf, int size, int offset){
    // Read inode
    // Locate offset
    // Write buffer to disk
}