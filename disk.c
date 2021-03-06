#include "disk.h"


int allocate_disk(const char* filename){
	//add_0 = (void*) malloc(1073741824 * sizeof(char)); // allocate disk (1GB)
	
	add = open(filename, O_RDWR|O_CREAT, 0666);
	if(add == -1){
		perror("error in creating disk \n");
		return -1;	
	}
	return 0;
}
int free_disk(){
        //free(add_0);
	int status = close(add);
	if(status == -1){
		printf("error in freeing disk \n");
		return -1;
	}
	return 0;
}

int disk_read(void* out, unsigned int block_id){
        Hash_Node* h_node = hash_find(block_id);
	if(h_node == NULL){
		//void* Disk_Buffer = (void*)((char*)add_0 + block_id*4096);
        	//memcpy(out, Disk_Buffer, DB_SIZE);
		//list_add(block_id, Disk_Buffer,0);
		if(lseek(add, block_id*DB_SIZE,SEEK_SET) == -1){
			return -1;
		}
		if(read(add,out,DB_SIZE) == -1){
			return -1;
		}
		list_add(block_id, out, 0);
	}
	else{
		//we have it inside the buffer
		read_from_cache(out, h_node->buffer_id);
		list_prioritize(h_node->list_node);
	}
	return 0;
}
int disk_write(void* in, unsigned int block_id, int access_disk){
	if(access_disk!=0){
		lseek(add, block_id*DB_SIZE,SEEK_SET);
        	if( write(add, in, DB_SIZE) == -1){
                	printf("write failed \n");
                	return -1;
        	}
        	return 0;
	}

	Hash_Node* h_node = hash_find(block_id);
	if(h_node == NULL){
		list_add(block_id, in,1);
	}
	else{
		write_to_cache(in , h_node->buffer_id);
		h_node->list_node->dirty=1;
		list_prioritize(h_node->list_node);
	}
	return 0;
}

/*
 */

void sync(){
	List_Node* head = list_head;
	while(head!=NULL){
		if(head->dirty){
			if(cache_to_disk(head->buffer_id, head->block_id) == -1){
				printf("cache to disk in sync failed! \n");
				return;
			}
			head->dirty=0;
		}
		head = head->next;
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

int cache_to_disk(unsigned int buffer_id, unsigned int block_id){
        //void* Disk_Buffer = (void*)((char*)add_0 + block_id * 4096);
	void* Disk_Buffer = calloc(sizeof(char)*DB_SIZE, 1);
	void* Cache_Buffer = (void*)((char*)buffer_0 + buffer_id * DB_SIZE); 
        memcpy(Disk_Buffer,Cache_Buffer,DB_SIZE);
	
	lseek(add, block_id*DB_SIZE,SEEK_SET);
        if( write(add, Disk_Buffer, DB_SIZE) == -1){
		free(Disk_Buffer);
		printf("write failed \n");
		return -1;
	}
	free(Disk_Buffer);
	return 0;
}
void write_to_cache(void* in, unsigned int buffer_id){
        void* Cache_Buffer = (void*)((char*)buffer_0 + buffer_id*DB_SIZE);
        memcpy(Cache_Buffer, in, DB_SIZE);
}
void read_from_cache(void* out, unsigned int buffer_id){
        void* Cache_Buffer = (void*)((char*)buffer_0 + buffer_id*DB_SIZE);
        memcpy(out, Cache_Buffer, DB_SIZE);
}

void list_init(){
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
	List_Node* curr = list_head;
	List_Node* next;
	while(curr!=NULL){
		next = curr->next;
		free(curr);
		curr = next;
	}
}
void list_add(unsigned int block_id, void* buffer, int dirty){
	List_Node* end = list_tail;
	list_tail = list_tail->prev;
	list_tail->next = NULL;
	end->prev = NULL;

	if(end->in_hash){
		if(end->dirty){
			if(cache_to_disk(end->buffer_id, end->block_id)==-1){
				printf("list_add cache to disk failed \n");
				return;
			}
		}
		hash_delete(end->block_id);
	}
	
	write_to_cache(buffer, end->buffer_id);
	end->block_id = block_id;
	end->dirty = dirty;
	end->in_hash = 1;
	hash_insert(block_id,end->buffer_id,end);
	
	end->next = list_head;
	list_head->prev = end;
	list_head = end;
}
void list_prioritize(List_Node* node){
	if(node->prev==NULL){
		//first element
		return;
	}
	if(node->next == NULL){
		//last element
		list_tail = node->prev;
		node->prev->next = NULL;

		node->next = list_head;
		list_head->prev = node;
		list_head = node;
		return;
	}
	else{
		//in the middle
		node->prev->next = node->next;
		node->next->prev = node->prev;

		node->prev = NULL;
		node->next = list_head;

		list_head->prev = node;
		list_head = node;
		return;
	}

}

/*
 */
void hash_init(){
	hash_table = (table*)malloc(sizeof(table));
	hash_table->list = (Hash_Node**)malloc(BUFFER_NUM * sizeof(Hash_Node*));
	for(int i=0; i<BUFFER_NUM; i++){
		hash_table->list[i]=NULL;
	}
}
void hash_free(){
	for(int i=0; i<BUFFER_NUM; i++){
		Hash_Node* curr = hash_table->list[i];
		Hash_Node* prev;
		while(curr!=NULL){
			prev = curr;
			curr = curr->next;
			free(prev);
		}
	}
	free(hash_table->list);
	free(hash_table);
	
}
int hash_func(unsigned int block_id){
	return block_id % BUFFER_NUM;
}
Hash_Node* hash_find(unsigned int block_id){
	int index = hash_func(block_id);
	Hash_Node* l = hash_table->list[index];
	Hash_Node* temp = l;
	while(temp!=NULL){
		if(temp->block_id == block_id){
			return temp;
		}
		temp = temp->next;
	}	
	return NULL;
}
int hash_insert(unsigned int block_id,unsigned int buffer_id, List_Node* node){
	int index = hash_func(block_id);
	Hash_Node* l = hash_table->list[index];
	Hash_Node* temp = l;
	while(temp!=NULL){
		if(temp->block_id == block_id){
			//it's already inside the hash table
			return -1;
		}
		temp=temp->next;
	}
	Hash_Node* new_node = (Hash_Node*)malloc(sizeof(Hash_Node));
	new_node->buffer_id = buffer_id;
	new_node->block_id = block_id;
	new_node->list_node = node;
	new_node->next = hash_table->list[index];
	hash_table->list[index] = new_node;
	
	

	return 1;
}
int hash_delete(unsigned int block_id){
	int index = hash_func(block_id);
        Hash_Node* l = hash_table->list[index];
        
	Hash_Node* temp = l;
	Hash_Node* prev = NULL;

	if(temp!=NULL && temp->block_id == block_id){
		hash_table->list[index] = temp->next;
		free(temp);
		return 1;
	}
	
        while(temp!=NULL){
                if(temp->block_id == block_id){
			prev->next = temp->next;
			free(temp);
                        return 1;
                }

		prev = temp;
                temp=temp->next;
        }
	
	return -1; //not found
}



