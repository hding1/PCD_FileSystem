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
    char block[DB_SIZE];
    memset(block, 0, DB_SIZE);
    for(int i = 0; i < 32; i++){
        memcpy(block + i * INODE_SIZE, &node, sizeof(node));
    }
    for(unsigned int bid = ILIST_BID; bid < ILIST_BID + NUM_INODE / (DB_SIZE / INODE_SIZE); bid++){
        db_write(block, bid);
    }

    return 0;
}


/**********************************Helper function****************************************/

int find_free_inode(){

    // Bring the bitmap from disk to memory
    unsigned short bitmap[NUM_INODE];

    int status = db_read(bitmap, BITMAP_BID);
    if(status < 0)
        return status;

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
    db_write(block, bid);

    return 0;
}

int free_indblo_by_bid(unsigned int bid){
    if(bid == 0) return 0;
    char block[DB_SIZE];
    db_read(block, bid);
    unsigned int bids[INDIR_ID_NUM];
    memcpy(bids, block, DB_SIZE);
    for(int i = 0; i < DIR_ID_NUM; i++){
        db_free(bids[i]);
    }
    db_free(bid);
    return 0;
}

int free_dindblo_by_bid(unsigned int bid){
    if(bid == 0) return 0;
    char block[DB_SIZE];
    db_read(block, bid);
    unsigned int bids[INDIR_ID_NUM];
    memcpy(bids, block, DB_SIZE);
    for(int i = 0; i < INDIR_ID_NUM; i++){
        free_indblo_by_bid(bids[i]);
    }
    db_free(bid);
    return 0;
}

int free_tindblo_by_bid(unsigned int bid){
    if(bid == 0) return 0;
    char block[DB_SIZE];
    db_read(block, bid);
    unsigned int bids[INDIR_ID_NUM];
    memcpy(bids, block, DB_SIZE);
    for(int i = 0; i < INDIR_ID_NUM; i++){
        free_dindblo_by_bid(bids[i]);
    }
    db_free(bid);
    return 0;
}

unsigned int find_block_by_num(unsigned int inum, unsigned int num){
    inode* target_node = find_inode_by_inum(inum);
    char block[DB_SIZE];
    unsigned int bid;
    unsigned int ind_bid;
    unsigned int dind_bid;
    unsigned int index;
    unsigned int d_index;
    unsigned int t_index;
    unsigned int ind_block[INDIR_ID_NUM];
    unsigned int dind_block[INDIR_ID_NUM];
    unsigned int tind_block[INDIR_ID_NUM];
    if(num < DIR_ID_NUM){
        bid = target_node->direct_blo[num];
    }else if(num < DIR_ID_NUM + INDIR_ID_NUM){
        index = num - DIR_ID_NUM;
        db_read(block, target_node->single_ind);
        memcpy(ind_block, block, DB_SIZE);
        bid = ind_block[index];
    }else if(num < DIR_ID_NUM + INDIR_ID_NUM + D_INDIR_ID_NUM){
        d_index = (num - DIR_ID_NUM - INDIR_ID_NUM) / INDIR_ID_NUM;
        index = (num - DIR_ID_NUM - INDIR_ID_NUM) % INDIR_ID_NUM;
        db_read(block, target_node->double_ind);
        memcpy(dind_block, block, DB_SIZE);
        ind_bid = dind_block[d_index];
        db_read(block, ind_bid);
        memcpy(ind_block, block, DB_SIZE);
        bid = ind_block[index];
    }else if(num < DIR_ID_NUM + INDIR_ID_NUM + D_INDIR_ID_NUM + T_INDIR_ID_NUM){
        t_index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) / D_INDIR_ID_NUM;
        d_index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) / INDIR_ID_NUM;
        index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) % INDIR_ID_NUM;
        db_read(block, target_node->triple_ind);
        memcpy(tind_block, block, DB_SIZE);
        dind_bid = tind_block[t_index];
        db_read(block, dind_bid);
        memcpy(dind_block, block, DB_SIZE);
        ind_bid = dind_block[d_index];
        db_read(block, ind_bid);
        memcpy(ind_block, block, DB_SIZE);
        bid = dind_block[index];
    }else{
        return -1;
    }
    free(target_node);
    return bid;
}

int write_block_by_num(unsigned int inum, unsigned int num, char* block){
    int bid = find_block_by_num(inum, num);
    if(bid == -1) return -1;
    db_write(block, bid);
    return bid;
}

int add_block(unsigned int inum){
    unsigned int newid = db_allocate();
    inode* target_node = find_inode_by_inum(inum);
    int num = target_node->size / DB_SIZE;
    if(target_node->size % DB_SIZE != 0) num++;

    char block[DB_SIZE];
    unsigned int ind_bid;
    unsigned int dind_bid;
    unsigned int tind_bid;
    unsigned int index;
    unsigned int d_index;
    unsigned int t_index;
    unsigned int ind_block[INDIR_ID_NUM];
    unsigned int dind_block[INDIR_ID_NUM];
    unsigned int tind_block[INDIR_ID_NUM];

    if(num < DIR_ID_NUM){
        target_node->direct_blo[num] = newid;
        write_inode_to_disk(inum, target_node);

    }else if(num < DIR_ID_NUM + INDIR_ID_NUM){
        index = num - DIR_ID_NUM;
        if(index == 0){
            ind_bid = db_allocate();
            target_node->single_ind = ind_bid;
            db_read(block, ind_bid);
            memcpy(ind_block, block, DB_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, DB_SIZE);
            db_write(block, ind_bid);
            write_inode_to_disk(inum, target_node);
        }else{
            db_read(block, target_node->single_ind);
            memcpy(ind_block, block, DB_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, DB_SIZE);
            db_write(block, target_node->single_ind);
        } 
        
    }else if(num < DIR_ID_NUM + INDIR_ID_NUM + D_INDIR_ID_NUM){
        d_index = (num - DIR_ID_NUM - INDIR_ID_NUM) / DB_SIZE;
        index = (num - DIR_ID_NUM - INDIR_ID_NUM) % DB_SIZE;
        if(d_index == 0 && index == 0){
            dind_bid = db_allocate();
            target_node->double_ind = dind_bid;
            db_read(block, dind_bid);
            memcpy(dind_block, block, DB_SIZE);
            ind_bid = db_allocate();
            dind_block[d_index] = ind_bid;
            db_read(block, ind_bid);
            memcpy(ind_block, block, DB_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, DB_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, DB_SIZE);
            db_write(block, dind_bid);
            write_inode_to_disk(inum, target_node);
        }else if(index == 0){
            db_read(block, target_node->double_ind);
            memcpy(dind_block, block, DB_SIZE);
            ind_bid = db_allocate();
            dind_block[d_index] = ind_bid;
            db_read(block, ind_bid);
            memcpy(ind_block, block, DB_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, DB_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, DB_SIZE);
            db_write(block, target_node->double_ind);
        }else{
            db_read(block, target_node->double_ind);
            memcpy(dind_block, block, DB_SIZE);
            ind_bid = dind_block[d_index];
            db_read(block, ind_bid);
            memcpy(ind_block, block, DB_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, DB_SIZE);
            db_write(block, ind_bid);
        }

    }else if(num < DIR_ID_NUM + INDIR_ID_NUM + D_INDIR_ID_NUM + T_INDIR_ID_NUM){
        t_index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) / D_INDIR_ID_NUM;
        d_index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) / INDIR_ID_NUM;
        index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) % INDIR_ID_NUM;
        if(t_index == 0 && d_index == 0 && index == 0){
            tind_bid = db_allocate();
            target_node->triple_ind = tind_bid;
            db_read(block, tind_bid);
            memcpy(tind_block, block, DB_SIZE);
            dind_bid = db_allocate();
            tind_block[t_index] = dind_bid;
            db_read(block, dind_bid);
            memcpy(dind_block, block, DB_SIZE);
            ind_bid = db_allocate();
            dind_block[d_index] = ind_bid;
            db_read(block, ind_bid);
            memcpy(ind_block, block, DB_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, DB_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, DB_SIZE);
            db_write(block, dind_bid);
            memcpy(block, tind_block, DB_SIZE);
            db_write(block, tind_bid);
            write_inode_to_disk(inum, target_node);
        }else if(d_index == 0 && index == 0){
            db_read(block, target_node->triple_ind);
            memcpy(tind_block, block, DB_SIZE);
            dind_bid = db_allocate();
            tind_block[t_index] = dind_bid;
            db_read(block, dind_bid);
            memcpy(dind_block, block, DB_SIZE);
            ind_bid = db_allocate();
            dind_block[d_index] = ind_bid;
            db_read(block, ind_bid);
            memcpy(ind_block, block, DB_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, DB_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, DB_SIZE);
            db_write(block, dind_bid);
            memcpy(block, tind_block, DB_SIZE);
            db_write(block, target_node->triple_ind);
        }else if(index == 0){
            db_read(block, target_node->triple_ind);
            memcpy(tind_block, block, DB_SIZE);
            dind_bid = tind_block[t_index];
            db_read(block, dind_bid);
            memcpy(dind_block, block, DB_SIZE);
            ind_bid = db_allocate();
            dind_block[d_index] = ind_bid;
            db_read(block, ind_bid);
            memcpy(ind_block, block, DB_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, DB_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, DB_SIZE);
            db_write(block, dind_bid);
        }else{
            db_read(block, target_node->triple_ind);
            memcpy(tind_block, block, DB_SIZE);
            dind_bid = tind_block[t_index];
            db_read(block, dind_bid);
            memcpy(dind_block, block, DB_SIZE);
            ind_bid = dind_block[d_index];
            db_read(block, ind_bid);
            memcpy(ind_block, block, DB_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, DB_SIZE);
            db_write(ind_block, ind_bid);
        }

    }else{
        return -1;
    }
    free(target_node);
    return 0;
}


/******************************************Inode opertaions***********************************************/

int inode_allocate(){

    // Find a opening in the bitmap
    unsigned int inum = find_free_inode();
    if(inum == -1) return -1;   // Error

    // Get the inode, set initial values
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;  // Error
    target_node->mode = 666;
    target_node->last_accessed = time(NULL);
    target_node->last_modified = time(NULL);
    target_node->link_count = 1;

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
    if(num_blo < DIR_ID_NUM){
        for(int i = 0; i < num_blo; i++){
            db_free(target_node->direct_blo[i]);
        }
    }else if(num_blo < INDIR_ID_NUM + DIR_ID_NUM){
        for(int i = 0; i < num_blo; i++){
            db_free(target_node->direct_blo[i]);
        }
        free_indblo_by_bid(target_node->single_ind);
    }else if(num_blo < D_INDIR_ID_NUM + INDIR_ID_NUM + DIR_ID_NUM){
        for(int i = 0; i < num_blo; i++){
            db_free(target_node->direct_blo[i]);
        }
        free_indblo_by_bid(target_node->single_ind);
        free_dindblo_by_bid(target_node->double_ind);
    }else if(num_blo < T_INDIR_ID_NUM + D_INDIR_ID_NUM + INDIR_ID_NUM + DIR_ID_NUM){
        for(int i = 0; i < num_blo; i++){
            db_free(target_node->direct_blo[i]);
        }
        free_indblo_by_bid(target_node->single_ind);
        free_dindblo_by_bid(target_node->double_ind);
        free_tindblo_by_bid(target_node->triple_ind);
    }else{
        return -1;
    }

    // Mark the inode free in bitmap
    unsigned short bitmap[NUM_INODE];
    db_read(bitmap, BITMAP_BID);
    bitmap[inum] = 0;
    db_write(bitmap, BITMAP_BID);

    free(target_node);
    return 0; // If success
}

int inode_read_mode(unsigned int inum, mode_t* mode_out){

    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;   // Error
    *mode_out = target_node->mode;
    free(target_node);

    return 0; // If success
}

int inode_write_mode(unsigned int inum, mode_t mode_in){
    
    // Modify mode of target inode
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1; // Error
    target_node->mode = mode_in;
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

int inode_reduce_link(unsigned int inum){
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;   // Error
    target_node->link_count =  target_node->link_count -1;
    if(target_node->link_count <= 0){
        inode_free(inum);
    }else{
        int status = write_inode_to_disk(inum, target_node);
        if(!status) return -1;
    }
    return 0;
}

unsigned int get_root_inum(){
    return ROOT_INUM;
}

int read_file(unsigned int inum, char* buf, int size, int offset){

    // Read inode
    inode * target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;
    if(buf == NULL) return -1;
    if(size < 0) return -1;
    if(offset < 0 || offset > target_node->size) return -1;

    // Locate offset 
    unsigned int start_num = offset / DB_SIZE;   // start block# in this inode
    unsigned int end_num = (offset + size) / DB_SIZE; // end block# in this inode
    unsigned int start_off = offset % DB_SIZE;  // start byte# in first block
    unsigned int toRead;
    if(size >= DB_SIZE - start_off){
        toRead = DB_SIZE - start_off;
    }else{
        toRead = size;
    }
    unsigned int buf_off = 0;

    unsigned int bid;
    char block[DB_SIZE];
    // Read disk to buf
    while(start_num <= end_num){
        bid = find_block_by_num(inum, start_num);
        db_read(block, bid);
        memcpy(buf + buf_off, block + start_off, toRead);
        buf_off += toRead;
        start_off = 0;
        start_num++;
        size -= toRead;
        if(size >= DB_SIZE){
            toRead = DB_SIZE;
        }
        else{
            toRead = size;
        }
    }
    free(target_node);
    return buf_off;
}

int write_file(unsigned int inum, const char* buf, int size, int offset){

    // Read inode
    inode * target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;
    if(buf == NULL) return -1;
    if(size < 0) return -1;
    if(offset < 0 || offset > target_node->size) return -1;

    if(offset + size > target_node->size){
        // allocate enough blocks
        int add_byte = offset + size - target_node->size;
        int blo_add = add_byte / DB_SIZE;
        int blo_off = add_byte % DB_SIZE;
        if(blo_off > 0) blo_add++;
        for(int i = 0; i < blo_add; i++){
            add_block(inum);
        }     
    }

    // Locate offset
    unsigned int start_num = offset / DB_SIZE;   // start block# in this inode
    unsigned int end_num = (offset + size) / DB_SIZE; // end block# in this inode
    unsigned int start_off = offset % DB_SIZE;  // start byte# in first block
    unsigned int toWrite;
    if(size >= DB_SIZE - start_off){
        toWrite = DB_SIZE - start_off;
    }else{ 
        toWrite = size;
    }
    unsigned int buf_off = 0;

    unsigned int bid;
    char block[4096];
    // Write buffer to disk
      while(start_num <= end_num){
        bid = find_block_by_num(inum, start_num);
        db_read(block, bid);
        memcpy(block + start_off, buf + buf_off, toWrite);
        write_block_by_num(inum, start_num, block);
        buf_off += toWrite;
        start_off = 0;
        start_num++;
        size -= toWrite;
        if(size >= DB_SIZE){
            toWrite = DB_SIZE;
        }
        else{
            toWrite = size;
        }
    }
    target_node->size += buf_off;
    write_inode_to_disk(inum, target_node);
    
    return buf_off;
}
