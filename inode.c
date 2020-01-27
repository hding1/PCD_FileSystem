// Responsible author(s): Zihao Zhang

#include <inode.h>

int inode_allocate(struct inode* node){
    node = (struct inode*) malloc(sizeof(struct inode));
    node->permission = 666;
    node->type = 0;
    node->UID = 0;      //STUB!! How to get UID, PID?
    node->GID = 0;      //STUB!!
    node->size = 4096;
    node->last_accessed = time(NULL);
    node->last_modified = time(NULL);

    for(int i = 0; i < DIRECT_BLKS_NUM; i++){
        node->direct_blo[i] = NULL;
    }
    node->single_ind = NULL;
    node->double_ind = NULL;
    node->triple_ind = NULL;

    return 0; // If success
}

int inode_free(struct inode* node){
    for(int i = 0; i < DIRECT_BLKS_NUM; i++){
        free(node->direct_blo[i]);
    }
    free(node->single_ind);
    free(node->double_ind);
    free(node->triple_ind);
    free(node);

    return 0; // If success
}

int inode_read(struct inode* node, char* disk_buffer){
    int offset = 0;
    offset += sprintf(disk_buffer + offset, "%d", node->permission);
    disk_buffer[offset++] = ";";
    offset += sprintf(disk_buffer + offset, "%d", node->type);
    disk_buffer[offset++] = ";";
    offset += sprintf(disk_buffer + offset, "%d", node->UID);
    disk_buffer[offset++] = ";";
    offset += sprintf(disk_buffer + offset, "%d", node->GID);
    disk_buffer[offset++] = ";";
    offset += sprintf(disk_buffer + offset, "%d", node->size);
    disk_buffer[offset++] = ";";
    offset += sprintf(disk_buffer + offset, "%ld", node->last_accessed);
    disk_buffer[offset++] = ";";
    offset += sprintf(disk_buffer + offset, "%ld", node->last_modified);
    disk_buffer[offset++] = ";";
    for(int i = 0; i < DIRECT_BLKS_NUM; i++){
        offset += sprintf(disk_buffer + offset, "%p", (void*) node->direct_blo[i]);
        disk_buffer[offset++] = ";";
    }
    offset += sprintf(disk_buffer + offset, "%p", (void*) node->single_ind);
    disk_buffer[offset++] = ";";
    offset += sprintf(disk_buffer + offset, "%p", (void*) node->double_ind);
    disk_buffer[offset++] = ";";
    offset += sprintf(disk_buffer + offset, "%p", (void*) node->triple_ind);

    return 0; // If success
}

int inode_write(struct inode* node, char* disk_buffer){
   
}

int inode_list_init(struct inode* list){

}