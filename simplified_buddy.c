#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int heap[1024];

void initialise_memory(){
	memset(heap, -1 , sizeof(heap));	//Filling all entries with -1 initially
		
	int i = 0; 		//root index
	heap[i] = 512; 	//One 512 block and it's starting address is 512
	i = 2*i + 1;	//Starting index of next level i.e., block of size 256
	heap[i] = 256;	//One 256 block and it's starting address is 256
	i = 2*i + 1;	//Starting index of next level i.e., block of size 128
	heap[i] = 128;	//One 128 block and it's starting address is 128
}

void print_memory_freelist(){
	int i = 0,size = 512, index = 1, temp = 0;
	printf("*****	Freelist for blocks of size 512	*****\n");
	printf("%d\n", heap[0]);
	for(i = 1; i < 127; i++){
		if(i == index) {
			temp = 0;
			size = size/2;
			index = 2 * index + 1;
			printf("\n*****	Freelist for blocks of size %d 	*****\n", size);
		}
		printf("%d\t ", heap[i]);
		temp++;
	}
}

int ceil2(int size){
	int temp = 8; //smallest block size allowed
	while(temp < 512 && temp < size){
		temp = temp * 2;
	}
	return temp;
}

int get_free_list(int size){
	int index = 0, current_size = 512;
	while(current_size != size){
		index = 2*index + 1;
		current_size = current_size / 2;
	}
	return index;
}

int find_free_list_size(int size){
	return 1024/(2*size);
}

void add_to_list(int free_list, int addr){
	int temp = free_list;
	while(heap[temp] != -1){
		temp++;
	}
	heap[temp] = addr;
}

void remove_from_list(int free_list, int free_list_size, int addr) {
	int temp = free_list;
	while(temp < free_list + free_list_size && heap[temp] != addr){
		temp++;
	}
	if(temp != free_list + free_list_size){
		temp++;
		while(temp < free_list + free_list_size) {
			heap[temp - 1] = heap[temp];
			temp++;
		}
		heap[temp - 1] = -1;	
	}
}

int get_free_block(int free_list,int list_size){
	int temp = free_list,temp1;
	while(temp < free_list + list_size && heap[temp] != -1){
		temp++;
	}
	temp--;
	temp1 = heap[temp];
	heap[temp] = -1;
	return temp1;
}

int split_rec(int current_list, int list_size, int current_size, int required_size){
	if(current_list < 0) {
		return 0;
	}
	if(heap[current_list] == -1 && !split_rec((current_list - 1)/2, list_size / 2, current_size*2, required_size)) {
		return 0;
	}
	if(required_size == current_size && heap[current_list] != -1) {
		return 1;
	}
	int temp = get_free_block(current_list,list_size);
	add_to_list(2*current_list+1, temp + current_size/2);
	add_to_list(2*current_list+1, temp);
	return 1;
}

int allocate(int size){
	
	if(size + 1 > 512){
		return -1;
	}

	int temp,ceil_size, req_free_list,req_free_list_size,start_addr;

	ceil_size = ceil2(size + 1);
	req_free_list = get_free_list(ceil_size);
	req_free_list_size = find_free_list_size(ceil_size);

	if(heap[req_free_list] == -1){
		temp = split_rec(req_free_list, req_free_list_size, ceil_size, ceil_size);
		if(temp == 0){
			return -1;
		}		
	}

	temp = req_free_list;
	while(temp < req_free_list + req_free_list_size && heap[temp] != -1){
		temp = temp + 1;
	}

	temp--;
	start_addr = heap[temp];
	heap[temp] = -1;
	heap[start_addr] = size;
	return start_addr;
}

void free_block(int start_addr,int size){
	int i;
	for(i=start_addr;i<start_addr + size;i++){
		heap[i] = -1;
	}
}

int find_free_buddy(int start_addr, int ceil_size) {
	int buddy;
	if(start_addr % (2 * ceil_size) == 0) {
		buddy = start_addr + ceil_size;	
	}
	else {
		buddy = start_addr - ceil_size;
	}
	int free_list = get_free_list(ceil_size);
	int free_list_size = find_free_list_size(ceil_size);
	int i = free_list;
	while(i < free_list + free_list_size){
		if(heap[i] == buddy){
			return i;
		}
		i++;
	}
	return -1;
}

int minimum(int x, int y) {
	return x < y? x : y;
}

void merge_buddies(int start_addr, int ceil_size) {
	if(ceil_size > 512) {
		return;
	}
	int buddy_free_index = find_free_buddy(start_addr, ceil_size);
	int free_list = get_free_list(ceil_size);
	int free_list_size = find_free_list_size(ceil_size);
	if(buddy_free_index == -1){
		add_to_list(free_list, start_addr);
		return;
	}
	else {
		int parent_free_list = get_free_list(2*ceil_size);
		int new_start_addr = minimum(heap[buddy_free_index], start_addr);
		remove_from_list(free_list, free_list_size, heap[buddy_free_index]);
		merge_buddies(new_start_addr, 2 * ceil_size);
	}
}

int deallocate(int start_addr){
	int size = heap[start_addr];
	int ceil_size,current;

	ceil_size = ceil2(size + 1);

	free_block(start_addr,ceil_size);

	merge_buddies(start_addr, ceil_size);
}


int main(){
	initialise_memory();
	int temp = allocate(510);
	deallocate(temp);
	print_memory_freelist();
	return 0;
}
