// Responsible author(s): Dennis

#include "inode.h"


/*********************************Init functions************************************/
int inode_bitmap_init(){

    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
	unsigned int NUM_INODE = super->MAX_NUM_INODE;
    unsigned int BITMAP_BID = super->START_BITMAP;
    free(super);

    // Create a bitmap of size NUM_INODE, set all inodes to be free
    unsigned short bitmap[NUM_INODE];
    for(int i = 0; i < NUM_INODE; i++){
        bitmap[i] = 0;
    }

    // Wirte the bitmap to disk
    if(db_write(bitmap, BITMAP_BID) == -1){
        return -1;  // bitmap init failed
    }  

    return 0; 
}

int inode_list_init(){

    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
	unsigned int NUM_INODE  =  super->MAX_NUM_INODE;
    unsigned int DIR_ID_NUM = super->DIR_ID_NUM;
    unsigned int INODE_SIZE = super->INODE_SIZE;
    unsigned int ILIST_BID = super->START_ILIST;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

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

    // Allocate the 4rd to 131th block (block id =3, 130) for inode list
    char block[BLOCK_SIZE];
    memset(block, 0, BLOCK_SIZE);
    for(int i = 0; i < BLOCK_SIZE / INODE_SIZE; i++){
        memcpy(block + i * INODE_SIZE, &node, sizeof(node));
    }
    for(unsigned int bid = ILIST_BID; bid < ILIST_BID + NUM_INODE / (BLOCK_SIZE / INODE_SIZE); bid++){
        if(db_write(block, bid) == -1)
            return -1; // ilist init failed  
    }

    return 0;
}


/**********************************Helper function****************************************/

int find_free_inode(){

    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
	unsigned int NUM_INODE  =  super->MAX_NUM_INODE;
    unsigned int BITMAP_BID = super->START_BITMAP;
    free(super);

    // Bring the bitmap from disk to memory
    unsigned short bitmap[NUM_INODE];

    db_read(bitmap, BITMAP_BID);

    for(int i = 0; i < NUM_INODE; i++){
        if(bitmap[i] == 0){
            bitmap[i] = 1;
            db_write(bitmap, BITMAP_BID);
            return i;   // Return the inum of the free inode
        }
    }
    return -1; // If unable to find a free inode
}

inode* find_inode_by_inum(unsigned int inum){

    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return NULL;
	unsigned int NUM_INODE  =  super->MAX_NUM_INODE;
    unsigned int ROOT_INUM = super->ROOT_INUM;
    unsigned int BITMAP_BID = super->START_BITMAP;
    unsigned int ILIST_BID = super->START_ILIST;
    unsigned int INODE_SIZE = super->INODE_SIZE;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    if(inum < ROOT_INUM || inum > NUM_INODE - 1){
        return NULL;
    }
    char block[BLOCK_SIZE];
    unsigned short bitmap[NUM_INODE];
    db_read(block, BITMAP_BID);
    memcpy(bitmap, block, BLOCK_SIZE);
    if(bitmap[inum] == 0){
        return NULL;
    }
    // Read the block
    unsigned int bid = ILIST_BID + inum / (BLOCK_SIZE/INODE_SIZE);
    db_read(block, bid);
    // Read the inode
    inode* node = (inode*) malloc(sizeof(inode));
    unsigned offset = inum % (BLOCK_SIZE/INODE_SIZE);
    char* ptr = (char*) (block + offset * INODE_SIZE);    // Find the inode in the block
    memcpy(node, ptr, sizeof(inode));

    return node;
}

int write_inode_to_disk(unsigned int inum, inode* target_node){
    
    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
	unsigned int NUM_INODE  =  super->MAX_NUM_INODE;
    unsigned int ROOT_INUM = super->ROOT_INUM;
    unsigned int ILIST_BID = super->START_ILIST;
    unsigned int INODE_SIZE = super->INODE_SIZE;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    if(inum < ROOT_INUM || inum > NUM_INODE - 1){
        return -1;
    }
    if(target_node == NULL) return -1;

    char block[BLOCK_SIZE];
    unsigned int bid = ILIST_BID + inum / (BLOCK_SIZE/INODE_SIZE);
    db_read(block, bid);
    unsigned offset = inum % (BLOCK_SIZE/INODE_SIZE);
    char* node_ptr = (char*) (block + offset * INODE_SIZE);
    memcpy(node_ptr, target_node, sizeof(inode));
    db_write(block, bid);

    return 0;
}

int free_indblo_by_bid(unsigned int bid){
    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
    unsigned int INDIR_ID_NUM = super->INDIR_ID_NUM;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    if(bid == 0) return 0;  // Unused
    char block[BLOCK_SIZE];
    db_read(block, bid);
    unsigned int bids[INDIR_ID_NUM];
    memcpy(bids, block, BLOCK_SIZE);
    for(int i = 0; i < INDIR_ID_NUM; i++){
        if(bids[i] == 0) break;  // Unused
        db_free(bids[i]);
        //printf("free_indblo_by_bid: indirect data bid = %d\n", bids[i]);
    }
    db_free(bid);
    //printf("free_indblo_by_bid: indirect bid = %d\n", bid);
    return 0;
}

int free_dindblo_by_bid(unsigned int bid){
    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
    unsigned int INDIR_ID_NUM = super->INDIR_ID_NUM;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    if(bid == 0) return 0;  // Unused
    char block[BLOCK_SIZE];
    db_read(block, bid);
    unsigned int bids[INDIR_ID_NUM];
    memcpy(bids, block, BLOCK_SIZE);
    for(int i = 0; i < INDIR_ID_NUM; i++){
        if(bids[i] == 0) break; // Unused
        free_indblo_by_bid(bids[i]);
    }
    db_free(bid);
    return 0;
}

int free_tindblo_by_bid(unsigned int bid){
    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
    unsigned int INDIR_ID_NUM = super->INDIR_ID_NUM;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    if(bid == 0) return 0;  // Unused
    char block[BLOCK_SIZE];
    db_read(block, bid);
    unsigned int bids[INDIR_ID_NUM];
    memcpy(bids, block, BLOCK_SIZE);
    for(int i = 0; i < INDIR_ID_NUM; i++){
        if(bids[i] == 0) break;
        free_dindblo_by_bid(bids[i]);
    }
    db_free(bid);
    return 0;
}

unsigned int find_block_by_num(unsigned int inum, unsigned int num){
    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
    unsigned int DIR_ID_NUM = super->DIR_ID_NUM;
    unsigned int INDIR_ID_NUM = super->INDIR_ID_NUM;
    unsigned int D_INDIR_ID_NUM = super->D_INDIR_ID_NUM;
    unsigned int T_INDIR_ID_NUM = super->T_INDIR_ID_NUM;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    inode* target_node = find_inode_by_inum(inum);
    char block[BLOCK_SIZE];
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
        memcpy(ind_block, block, BLOCK_SIZE);
        bid = ind_block[index];
    }else if(num < DIR_ID_NUM + INDIR_ID_NUM + D_INDIR_ID_NUM){
        d_index = (num - DIR_ID_NUM - INDIR_ID_NUM) / INDIR_ID_NUM;
        index = (num - DIR_ID_NUM - INDIR_ID_NUM) % INDIR_ID_NUM;
        db_read(block, target_node->double_ind);
        memcpy(dind_block, block, BLOCK_SIZE);
        ind_bid = dind_block[d_index];
        db_read(block, ind_bid);
        memcpy(ind_block, block, BLOCK_SIZE);
        bid = ind_block[index];
    }else if(num < DIR_ID_NUM + INDIR_ID_NUM + D_INDIR_ID_NUM + T_INDIR_ID_NUM){
        t_index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) / D_INDIR_ID_NUM;
        d_index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) / INDIR_ID_NUM;
        index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) % INDIR_ID_NUM;
        db_read(block, target_node->triple_ind);
        memcpy(tind_block, block, BLOCK_SIZE);
        dind_bid = tind_block[t_index];
        db_read(block, dind_bid);
        memcpy(dind_block, block, BLOCK_SIZE);
        ind_bid = dind_block[d_index];
        db_read(block, ind_bid);
        memcpy(ind_block, block, BLOCK_SIZE);
        bid = dind_block[index];
    }else{
        return -1;
    }
    free(target_node);
    return bid;
}

int write_block_by_num(unsigned int inum, unsigned int num, char* block){
    int bid = find_block_by_num(inum, num);
    //printf("writeblockbynum: bid = %d\n", bid);
    if(bid == -1) return -1;
    db_write(block, bid);
    return bid;
}

int add_block(unsigned int inum){
    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
    unsigned int DIR_ID_NUM = super->DIR_ID_NUM;
    unsigned int INDIR_ID_NUM = super->INDIR_ID_NUM;
    unsigned int D_INDIR_ID_NUM = super->D_INDIR_ID_NUM;
    unsigned int T_INDIR_ID_NUM = super->T_INDIR_ID_NUM;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    int newid = db_allocate();
    //printf("inode add block newid = %u\n", newid);
    if(newid == -1) return -1;
    inode* target_node = find_inode_by_inum(inum);
    int num = target_node->size / BLOCK_SIZE;
    if(target_node->size % BLOCK_SIZE != 0) num++;

    char block[BLOCK_SIZE];
    int ind_bid;
    int dind_bid;
    int tind_bid;
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
            if(ind_bid == -1) return -1;
            target_node->single_ind = ind_bid;
            db_read(block, ind_bid);
            memcpy(ind_block, block, BLOCK_SIZE);
            ind_block[0] = newid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                ind_block[i] = 0;
            }
            memcpy(block, ind_block, BLOCK_SIZE);
            db_write(block, ind_bid);
            write_inode_to_disk(inum, target_node);
        }else{
            db_read(block, target_node->single_ind);
            memcpy(ind_block, block, BLOCK_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, BLOCK_SIZE);
            db_write(block, target_node->single_ind);
        } 
        
    }else if(num < DIR_ID_NUM + INDIR_ID_NUM + D_INDIR_ID_NUM){
        d_index = (num - DIR_ID_NUM - INDIR_ID_NUM) / INDIR_ID_NUM;
        index = (num - DIR_ID_NUM - INDIR_ID_NUM) % INDIR_ID_NUM;
        if(d_index == 0 && index == 0){
            dind_bid = db_allocate();
            if(dind_bid == -1) return -1;
            target_node->double_ind = dind_bid;
            db_read(block, dind_bid);
            memcpy(dind_block, block, BLOCK_SIZE);
            ind_bid = db_allocate();
            if(ind_bid == -1) return -1;
            dind_block[0] = ind_bid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                dind_block[i] = 0;
            }
            db_read(block, ind_bid);
            memcpy(ind_block, block, BLOCK_SIZE);
            ind_block[0] = newid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                ind_block[i] = 0;
            }
            memcpy(block, ind_block, BLOCK_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, BLOCK_SIZE);
            db_write(block, dind_bid);
            write_inode_to_disk(inum, target_node);
        }else if(index == 0){
            db_read(block, target_node->double_ind);
            memcpy(dind_block, block, BLOCK_SIZE);
            ind_bid = db_allocate();
            if(ind_bid == -1) return -1;
            dind_block[d_index] = ind_bid;
            db_read(block, ind_bid);
            memcpy(ind_block, block, BLOCK_SIZE);
            ind_block[0] = newid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                ind_block[i] = 0;
            }
            memcpy(block, ind_block, BLOCK_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, BLOCK_SIZE);
            db_write(block, target_node->double_ind);
        }else{
            db_read(block, target_node->double_ind);
            memcpy(dind_block, block, BLOCK_SIZE);
            ind_bid = dind_block[d_index];
            db_read(block, ind_bid);
            memcpy(ind_block, block, BLOCK_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, BLOCK_SIZE);
            db_write(block, ind_bid);
        }

    }else if(num < DIR_ID_NUM + INDIR_ID_NUM + D_INDIR_ID_NUM + T_INDIR_ID_NUM){
        t_index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) / D_INDIR_ID_NUM;
        d_index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) / INDIR_ID_NUM;
        index = (num - DIR_ID_NUM - INDIR_ID_NUM - D_INDIR_ID_NUM) % INDIR_ID_NUM;
        if(t_index == 0 && d_index == 0 && index == 0){
            tind_bid = db_allocate();
            if(tind_bid == -1) return -1;
            target_node->triple_ind = tind_bid;
            db_read(block, tind_bid);
            memcpy(tind_block, block, BLOCK_SIZE);
            dind_bid = db_allocate();
            if(dind_bid == -1) return -1;
            tind_block[0] = dind_bid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                tind_block[i] = 0;
            }
            db_read(block, dind_bid);
            memcpy(dind_block, block, BLOCK_SIZE);
            ind_bid = db_allocate();
            if(ind_bid == -1) return -1;
            dind_block[0] = ind_bid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                dind_block[i] = 0;
            }
            db_read(block, ind_bid);
            memcpy(ind_block, block, BLOCK_SIZE);
            ind_block[0] = newid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                ind_block[i] = 0;
            }
            memcpy(block, ind_block, BLOCK_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, BLOCK_SIZE);
            db_write(block, dind_bid);
            memcpy(block, tind_block, BLOCK_SIZE);
            db_write(block, tind_bid);
            write_inode_to_disk(inum, target_node);
        }else if(d_index == 0 && index == 0){
            db_read(block, target_node->triple_ind);
            memcpy(tind_block, block, BLOCK_SIZE);
            dind_bid = db_allocate();
            if(dind_bid == -1) return -1;
            tind_block[t_index] = dind_bid;
            db_read(block, dind_bid);
            memcpy(dind_block, block, BLOCK_SIZE);
            ind_bid = db_allocate();
            if(ind_bid == -1) return -1;
            dind_block[0] = ind_bid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                dind_block[i] = 0;
            }
            db_read(block, ind_bid);
            memcpy(ind_block, block, BLOCK_SIZE);
            ind_block[0] = newid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                ind_block[i] = 0;
            }
            memcpy(block, ind_block, BLOCK_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, BLOCK_SIZE);
            db_write(block, dind_bid);
            memcpy(block, tind_block, BLOCK_SIZE);
            db_write(block, target_node->triple_ind);
        }else if(index == 0){
            db_read(block, target_node->triple_ind);
            memcpy(tind_block, block, BLOCK_SIZE);
            dind_bid = tind_block[t_index];
            db_read(block, dind_bid);
            memcpy(dind_block, block, BLOCK_SIZE);
            ind_bid = db_allocate();
            if(ind_bid == -1) return -1;
            dind_block[d_index] = ind_bid;
            db_read(block, ind_bid);
            memcpy(ind_block, block, BLOCK_SIZE);
            ind_block[0] = newid;
            for(int i = 1; i < INDIR_ID_NUM; i++){
                ind_block[i] = 0;
            }
            memcpy(block, ind_block, BLOCK_SIZE);
            db_write(block, ind_bid);
            memcpy(block, dind_block, BLOCK_SIZE);
            db_write(block, dind_bid);
        }else{
            db_read(block, target_node->triple_ind);
            memcpy(tind_block, block, BLOCK_SIZE);
            dind_bid = tind_block[t_index];
            db_read(block, dind_bid);
            memcpy(dind_block, block, BLOCK_SIZE);
            ind_bid = dind_block[d_index];
            db_read(block, ind_bid);
            memcpy(ind_block, block, BLOCK_SIZE);
            ind_block[index] = newid;
            memcpy(block, ind_block, BLOCK_SIZE);
            db_write(ind_block, ind_bid);
        }

    }else{
        return -1;
    }
    free(target_node);
    return newid;
}

unsigned long get_inode_size(unsigned int inum){
    inode* target_node = find_inode_by_inum(inum);
    unsigned long size = target_node->size;
    free(target_node);
    return size;
}

int set_inode_size(unsigned int inum, unsigned long size){
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;
    target_node->size = size;
    write_inode_to_disk(inum, target_node);
    free(target_node);
    return 0;
}


/******************************************Inode opertaions***********************************************/

int inode_allocate(){
    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
    unsigned int DIR_ID_NUM = super->DIR_ID_NUM;
    free(super);

    // Find a opening in the bitmap
    unsigned int inum = find_free_inode();
    if(inum == -1) return -1;   // Error

    // Get the inode, set initial values
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;  // Error
    target_node->mode = 0666;
    target_node->last_accessed = time(NULL);
    target_node->last_modified = time(NULL);
    target_node->link_count = 1;
    target_node->size = 0;

     char *p=getenv("USER");
    if(p==NULL) return EXIT_FAILURE;
    struct passwd *pw = getpwnam(p);
    target_node->UID = pw->pw_uid;
    target_node->GID = pw->pw_gid;

    for(int i = 0; i < DIR_ID_NUM; i++){
        target_node->direct_blo[i] = 0;
    }
    target_node->single_ind = 0;
    target_node->double_ind = 0;
    target_node->triple_ind = 0;
    
    // Write changes back to disk
    int status = write_inode_to_disk(inum, target_node);
    if(status) return -1;

    // Return the inum if success
    return inum; 
}

int inode_free(unsigned int inum){
    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
    unsigned int DIR_ID_NUM = super->DIR_ID_NUM;
    unsigned int INDIR_ID_NUM = super->INDIR_ID_NUM;
    unsigned int D_INDIR_ID_NUM = super->D_INDIR_ID_NUM;
    unsigned int T_INDIR_ID_NUM = super->T_INDIR_ID_NUM;
    unsigned int NUM_INODE = super->MAX_NUM_INODE;
    unsigned int BITMAP_BID = super->START_BITMAP;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    // Free data blocks used    
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;
    // Find how many blocks used
    int num_blo = target_node->size / BLOCK_SIZE;
    int offset = target_node->size % BLOCK_SIZE;
    if(offset != 0) num_blo++;
    // Free those blocks
    if(num_blo < DIR_ID_NUM){
        //printf("inode_free: free direct blocks!\n");
        for(int i = 0; i < num_blo; i++){
            db_free(target_node->direct_blo[i]);
        }
    }else if(num_blo < INDIR_ID_NUM + DIR_ID_NUM){
        //printf("inode_free: free direct + single indirect blocks!\n");
        for(int i = 0; i < DIR_ID_NUM; i++){
            db_free(target_node->direct_blo[i]);
            //printf("inode_free: free direct bid = %d\n", target_node->direct_blo[i]);
        }
        free_indblo_by_bid(target_node->single_ind);
    }else if(num_blo < D_INDIR_ID_NUM + INDIR_ID_NUM + DIR_ID_NUM){
        for(int i = 0; i < DIR_ID_NUM; i++){
            db_free(target_node->direct_blo[i]);
        }
        free_indblo_by_bid(target_node->single_ind);
        free_dindblo_by_bid(target_node->double_ind);
    }else if(num_blo < T_INDIR_ID_NUM + D_INDIR_ID_NUM + INDIR_ID_NUM + DIR_ID_NUM){
        for(int i = 0; i < DIR_ID_NUM; i++){
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
    if(status == -1) return -1;

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

int inode_reduce_link_count(unsigned int inum){
    inode* target_node = find_inode_by_inum(inum);
    if(target_node == NULL) return -1;   // Error
    target_node->link_count =  target_node->link_count -1;
    if(target_node->link_count <= 0){
        //printf("inode freed due to low link count!\n");
        inode_free(inum);
    }else{
        int status = write_inode_to_disk(inum, target_node);
        if(status == -1) return -1;
    }
    return 0;
}

unsigned int get_root_inum(){
    sb super;
    if(sb_read(&super) == -1){
        fprintf(stderr, "Error: sb read failed\n");
        return -1;
    }
    unsigned int ROOT_INUM = super.ROOT_INUM;
    fprintf(stderr, "Debug: ROOT INUM = %u\n", ROOT_INUM);
    return ROOT_INUM;
}

int read_file(unsigned int inum, char* buf, int size, int offset){
    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    // Read inode
    if(buf == NULL) return -1;
    if(size < 0) return -1;
    unsigned long inode_size = get_inode_size(inum);
    if(offset < 0 || offset > inode_size) return -1;

    // Locate offset 
    unsigned int start_num = offset / BLOCK_SIZE;   // start block# in this inode
    unsigned int end_num; // end block# in this inode
    if(offset + size > inode_size){
        end_num = inode_size / BLOCK_SIZE;
        size = inode_size;
    }else{
        end_num = (offset + size) / BLOCK_SIZE;
    }
    if((offset + size) % BLOCK_SIZE == 0) end_num--;
    unsigned int start_off = offset % BLOCK_SIZE;  // start byte# in first block
    unsigned int toRead;
    if(size >= BLOCK_SIZE - start_off){
        toRead = BLOCK_SIZE - start_off;
    }else{
        toRead = size;
    }
    unsigned int buf_off = 0;

    unsigned int bid;
    char block[BLOCK_SIZE];
    // Read disk to buf
    while(start_num <= end_num){
        bid = find_block_by_num(inum, start_num);
        db_read(block, bid);
        memcpy(buf + buf_off, block + start_off, toRead);
        buf_off += toRead;
        start_off = 0;
        start_num++;
        size -= toRead;
        if(size >= BLOCK_SIZE){
            toRead = BLOCK_SIZE;
        }
        else{
            toRead = size;
        }
    }
    return buf_off;
}

int write_file(unsigned int inum, const char* buf, int size, int offset){

    sb* super = (sb*) malloc(sizeof(sb));
    if(sb_read(super) == -1) return -1;
    unsigned int ROOT_INUM = super->ROOT_INUM;
    unsigned int NUM_INODE = super->MAX_NUM_INODE;
    unsigned int BLOCK_SIZE = super->blocksize;
    free(super);

    // Read inode
    if(inum < ROOT_INUM || inum > NUM_INODE - 1) return -1;
    if(buf == NULL) return -1;
    if(size < 0) return -1;
    unsigned long inode_size = get_inode_size(inum);
    if(offset < 0 || offset > inode_size) return -1;

    // Locate offset
    unsigned int start_num = offset / BLOCK_SIZE;   // start block# in this inode
    unsigned int end_num = (offset + size) / BLOCK_SIZE; // end block# in this inode
    if((offset + size) % BLOCK_SIZE == 0) end_num--;
    unsigned int start_off = offset % BLOCK_SIZE;  // start byte# in first block
    unsigned int toWrite;
    if(size >= BLOCK_SIZE - start_off){
        toWrite = BLOCK_SIZE - start_off;
    }else{ 
        toWrite = size;
    }
    unsigned int buf_off = 0;
    unsigned int bid;
    char block[BLOCK_SIZE];
    // Write buffer to disk
    while(start_num <= end_num){
        // add data block if necessary
        if(offset + buf_off + toWrite > inode_size){
           add_block(inum);
        }
        //printf("writefile: start_num = %d\n", start_num);
        bid = find_block_by_num(inum, start_num);
        //printf("writefile: bid = %d\n", bid);
        db_read(block, bid);
        memcpy(block + start_off, buf + buf_off, toWrite);
        //printf("writefile: block = %s\n", block);
        write_block_by_num(inum, start_num, block);
        if(offset + buf_off + toWrite > inode_size){
            inode_size += toWrite;
            set_inode_size(inum, inode_size);
        }
        buf_off += toWrite;
        start_off = 0;
        start_num++;
        size -= toWrite;
        if(size >= BLOCK_SIZE){
            toWrite = BLOCK_SIZE;
        }
        else{
            toWrite = size;
        }
    }
    
    //printf("writefile: inode size = %lu\n", get_inode_size(inum));
    
    return buf_off;
}
