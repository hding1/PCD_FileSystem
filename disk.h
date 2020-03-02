//author: Fuheng
#include <string.h>
#include <stdlib.h>
#include<stdio.h> 
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#ifndef DISK_H_
#define DISK_H_

#define DB_SIZE 4096
#define BUFFER_NUM 1000

//void* add_0;
int add;
void* buffer_0;
// Address of the first block is add_0
// Address of the first cache is buffer_0


int allocate_disk(const char* filename);
int free_disk();
int disk_read(void* out, unsigned int block_id);
int disk_write(void* in, unsigned int block_id);

/*
 */

void allocate_cache(); 
void deallocate_cache();

/*
 */

void sync();

/*
 */

int cache_to_disk(unsigned int buffer_id, unsigned int block_id);
void write_to_cache(void* in, unsigned int buffer_id);
void read_from_cache(void* out, unsigned int buffer_id);

typedef struct List_Node{
        unsigned int buffer_id;
        unsigned int block_id;
        struct List_Node* next;
        struct List_Node* prev;
        unsigned short dirty;
        unsigned short in_hash;
}List_Node;
List_Node* list_head;
List_Node* list_tail;
void list_init();
void list_free();
void list_add(unsigned int block_id, void* buffer, int dirty);
void list_prioritize(List_Node* node);

/*
 */

typedef struct Hash_Node{
        unsigned int buffer_id;
        unsigned int block_id;
	struct List_Node* list_node;
        struct Hash_Node* next;
}Hash_Node;

typedef struct table{
	struct Hash_Node** list; 
}table;

table* hash_table;
void hash_init();
void hash_free();
int hash_func(unsigned int block_id);
Hash_Node* hash_find(unsigned int block_id);
int hash_insert(unsigned int block_id, unsigned int buffer_id, List_Node* node);
int hash_delete(unsigned int block_id);



#endif
