#include "disk.h"


void allocate_disk(){
	add_0 = (void*) malloc(1073741824 * sizeof(char)); // allocate disk (1GB)
}
void free_disk(){
        free(add_0);
}

void disk_read(void* out, unsigned int block_id){
        Hash_Node* h_node = hash_find(block_id);
	if(h_node == NULL){
		void* Disk_Buffer = (void*)((char*)add_0 + block_id*4096);
        	memcpy(out, Disk_Buffer, DB_SIZE);
		list_add(block_id, Disk_Buffer);
	}
	else{
		//we have it inside the buffer
		read_from_cache(out, h_node->buffer_id);
	}
}
void disk_write(void* in, unsigned int block_id){
	Hash_Node* h_node = hash_find(block_id);
	if(h_node == NULL){
		list_add(block_id, in);
	}
	else{
		write_to_cache(in , h_node->buffer_id);
	}
}

/*
 */

void sync(){
	List_Node* head = list_head;
	while(head!=NULL){
		if(head->dirty){
			cache_to_disk(head->buffer_id, head->disk_id);
			head->dirty=0;
		}
	}
}

/*
 */

void allocate_cache(){
	buffer_0 = (void*)malloc(BUFFER_NUM*DB_SIZE*sizeof(char)); 
}
void deallocate_cache(){
	free(buffer_0);
}

/*
 */

void cache_to_disk(unsigned int buffer_id, unsigned int block_id){
        void* Disk_Buffer = (void*)((char*)add_0 + block_id * 4096);
	void* Cache_Buffer = (void*)((char*)buffer_0 + buffer_id * 4096); 
        memcpy(Disk_Buffer,Cache_Buffer,DB_SIZE);
}
void write_to_cache(void* in, unsigned int buffer_id){
        void* Cache_Buffer = (void*)((char*)buffer_0 + buffer_id*4096);
        memcpy(Cache_Buffer, in, DB_SIZE);
}
void read_from_cache(void* out, unsigned int buffer_id){
        void* Cache_Buffer = (void*)((char*)buffer_0 + buffer_id*4096);
        memcpy(out, Cache_Buffer, DB_SIZE);
}

void list_init(){
	list_head =(List_Node*)malloc(sizeof(List_Node));
	list_head = NULL;

	List_Node* head = list_head;
	
	for(unsigned int i=0; i<BUFFER_NUM; i++){
		List_Node* node =(List_Node*)malloc(sizeof(List_Node));
		node->buffer_id = i;
		node->in_hash = 0;
		node->dirty = 0;
		node->next = NULL;
		node->prev = head;

		if(head==NULL){
			list_head = node;
			head = node;
		}
		else{
			head->next = node;
			head = head->next;
	
		}
	}
	list_tail = head;

}

void list_free(){
	List_Node* temp;
	for(unsigned int i = 1; i<BUFFER_NUM; i++){
		temp = list_head;
		list_head = list_head->next;
		free(temp);
	}
	free(list_head);
}
void list_add(unsigned int block_id, void* buffer){
	List_Node* end = list_tail;
	list_tail = list_tail->prev;
	list_tail->next = NULL;
	end->prev = NULL;

	if(end->in_hash){
		hash_delete(end->buffer_id);
		if(end->dirty){
			cache_to_disk(end->buffer_id, end->block_id);
		}
	}
	
	write_to_cache(buffer, end->buffer_id);
	end->block_id = block_id;
	end->dirty = 0;
	end->in_hash = 1;
	hash_insert(block_id,end->buffer_id);
	
	end->next = list_head;
	list_head->prev = end;
	list_head = end;
}

/*
 */
void hash_init(){



}


