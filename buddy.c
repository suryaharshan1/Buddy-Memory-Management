#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Create sufficiently large memory space to handle requests
int heap[1024];

/*
 * @usage  : At the time of program initialisation 
 * @param  : NULL
 * @return : void 
 */
void initialise_memory(){
	memset(heap, -1 , sizeof(heap));	//Filling all entries with -1 initially
	
	//root node of the tree starts at index 0 of the heap
	//Ignoring blocks of size 1024 as they can never exist with freelists always residing in memory
	//Therefore, freelist of block size 512 starts at 0 and of size 1
	//Further levels can be accessed by (2 * prev_index + 1) and each of size (2 * prev_size)
	//Initially there are one 512 block, one 256 block, one 128 block in the memory starting at addresses (indexes) 512, 256, 128 respectively
	
	int i = 0; 		//root index
	
	/*
	 *	Iterative way to do the initialisation
	 *
	 *	int address = 512;
	 *	while(address >= 128){
	 *		heap[i] = address;
	 *		address /= 2;
	 * 		i = 2*i + 1;
	 *	}
	 *
	 */
	
	heap[i] = 512; 	//One 512 block and it's starting address is 512
	i = 2*i + 1;	//Starting index of next level i.e., block of size 256
	heap[i] = 256;	//One 256 block and it's starting address is 256
	i = 2*i + 1;	//Starting index of next level i.e., block of size 128
	heap[i] = 128;	//One 128 block and it's starting address is 128
}

/*
 * @usage  : For allocation of memory 
 * @param  : required size of memory block
 * @return : starting address of the memory block created 
 */

int allocate(int size) {
	/*
	 * One word is used to store the actual size of the structure in the memory
	 * Let this be the starting index of the block for simplicity
	 * So whenever a block of size 'x' is requested, the actual size of the block that is allocated is 'x+1'
	 */

	int start_addr;

	if(size > 511)	//Largest block that can be allocated is of size 512
		return -1;
	else if(size > 255){	//if size is in the range 256 - 511, 512 block of memory has to allocated
		if(heap[0] == -1){	//if 512 block is not free, abort
			return -1;
		}
		heap[heap[0]] = size;	//allocate 512 block of memory
		start_addr = heap[0];
		heap[0] = -1;			//remove it from 512 block freelist
		return start_addr;
	}
	int index = 0, parent = 0, current = 0, parent_size = 512, nearest_power = 512;
	while(nearest_power >= 8 && (size + 1) <= nearest_power/2){		//while nearest_power is not the nearest power and it is 8
		//Updating the parent if a free block is available
		if(heap[index] != -1){
			parent = index;
			parent_size = nearest_power;	
		}
		index = 2*index + 1;										//Take index to starting index to next freelist
		nearest_power /= 2;
	}
	if(heap[index] == -1 && heap[parent] == -1){
		//No free blocks available to accommodate the appropriate size
		return -1;
	}
	if(heap[index] == -1){
		//no free blocks of appropriate size are available
		//make free blocks of appropriate size by splitting the parent free blocks
		//Each freelist stimulates a stack
		//insertion and removal from the last for easy maintenance	
		current = parent;
		while(heap[current] > 0){current++;}	//Iterating to the top of the stack
		current--;					
		
		start_addr = heap[current];	//Taking the starting address of the freeblock from the stack
		heap[current] = -1;			//removing it from the stack
		
		do {
			parent = 2*parent + 1;	//Traversing down the tree filling the freelist to the appropriate size
			heap[parent] = start_addr + parent_size / 2; //adding the other free half to the free list
			parent_size /= 2;
		} while(parent != index);
		heap[start_addr] = size; 	//to know the size of the block that has been allocated starting from start_addr
		return start_addr;			//return the address of allocated block
	}
	else{
		//block of required size is available
		current = index;
		while(heap[current] > 0){current++;} 		//Iterating to the top of the stack
		current--;
		start_addr = heap[current];			//storing the start_addr
		heap[current] = -1;					//removing the block from free list
		
		heap[start_addr] = size; 	//to know the size of the block that has been allocated starting from start_addr
		return start_addr;			//return the address of allocated block	
	}
	
}


/*
 * @usage  : For deallocation of memory 
 * @param  : starting address of the memory block to be deallocated
 * @return : void
 */
 
void deallocate(int start_addr){
	int size = heap[start_addr];
	
	int nearest_power = 512, index = 0, current = 0;

	//Finding free list for block size "size"
	while(nearest_power >= 8 && (size + 1) <= nearest_power/2){		//while nearest_power is not the nearest power and it is 8
		index = 2*index + 1;										//Take index to starting index to next freelist
		nearest_power /= 2;
	}

	//free the block of memory occupied
	for(current = start_addr; current < start_addr + nearest_power; current++){
		heap[current] = -1;
	}

	int flag = 1;	//flag indicating the deallocation

	while(flag && index >= 0){
		//find buddy for the current freed block
		//an observation is used here to make this easy
		//a buddy with starting address x and of size y (y is a power of 2) with always hold the property x % y == 0
		//Hence, if x % (2*y) == 0 then the starting address of buddy is 'x + size' else 'x - size'
		int buddy = (start_addr % (2 * nearest_power) == 0) ?  start_addr + nearest_power : start_addr - nearest_power;

		//find buddy in the free list
		current = index;
		while(current < 2*index + 1 && heap[current] != buddy && heap[current] != -1) {
			current++;
		}
		if(heap[current] == -1 && current < 2*index + 1){			//Buddy is not found, hence add the current block to the free list
			heap[current] = start_addr;	
			flag = 0;
		}
		else if(heap[current] != -1 && current < 2*index + 1){		//Buddy found, merge them to form larger block
			//remove buddy from the current freelist
			while(current < 2*index && heap[current] != -1){
				heap[current] = heap[current + 1];
				current++;
			}
			heap[current] = -1;
			start_addr = buddy < start_addr ? buddy : start_addr;	//Update starting address and size of the larger free block	
			nearest_power *= 2;
			index = (index - 1)/2;									//Update index to traverse the tree bottom up to larger free block
		}
		else {
			printf("*****	Fatal Memory Error	*****\n");	//Deallocating failed due to some reason
			exit(1);
		}	
	}
	if(flag) {
		printf("*****	Fatal Memory Error	*****\n");	//Deallocating failed due to some reason
		exit(1);
	}				
}

/*
 * @usage  : For printing the memory free list
 * @param  : NULL
 * @return : void
 */
 
void print_memory_freelist(){
	int i = 0,size = 512, index = 1, temp = 0;
	printf("*****	Freelist for blocks of size 512	*****\n");
	printf("0\t%d\n", heap[0]);
	for(i = 1; i < 127; i++){
		if(i == index) {
			temp = 0;
			size = size/2;
			index = 2 * index + 1;
			printf("*****	Freelist for blocks of size %d 	*****\n", size);
		}
		printf("%d\t%d\n", temp, heap[i]);
		temp++;
	}
}

int main() {
	initialise_memory();
	print_memory_freelist();
	//Use allocate, deallocate, print_memory_freelist here
	//Sample
	/*
	  int mem1 = allocate(60);
	  if(mem1 == -1) {	
	  	//@TODO handle allocation failure	
	  }
	  int mem2 = allocate(150);
	  if(mem2 == -1) {	
	  	//@TODO handle allocation failure	
	  }
	  //Use mem1,mem2 here
	  dellocate(mem1);
	  dellocate(mem2);
	 */
	return 0;
}
