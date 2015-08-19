#include <stdio.h>
#include <string.h>

int heap[1024];

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

int allocate(int size) {

	/*
	 * One word is used to store the actual size of the structure in the memory
	 * Let this be the starting index of the block for simplicity
	 * So whenever a block of size 'x' is requested, the actual size of the block that is allocated is 'x+1'
	 */

	int start_addr;

	if(size > 511)	//Largest block that can be allocated is of size 512
		return -1;
	else if(size > 255){
		if(heap[0] == -1){
			return -1;
		}
		heap[heap[0]] = size;
		start_addr = heap[0];
		heap[0] = -1;
		return start_addr;
	}
	int index = 0, nearest_power = 512;
	while((size + 1)/nearest_power == 0){	//while nearest_power is not the nearest power
		index = 2*index + 1;	//Take index to starting index to next freelist
		nearest_power /= 2;
	}
}

int main() {
	return 0;
}