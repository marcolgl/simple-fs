#include "disk_driver.h"
#include "utility.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

#define DEBUG 0



/*

// this is stored in the 1st block of the disk

typedef struct {
  int num_blocks;
  int bitmap_blocks;   // how many blocks in the bitmap //AGG: + DiskHeader
  int bitmap_entries;  // how many bytes are needed to store the bitmap
  
  int free_blocks;     // free blocks
  int first_free_block;// first block index
} DiskHeader; 

typedef struct {
  DiskHeader* header; // mmapped
  char* bitmap_data;  // mmapped (bitmap)
  int fd; // for us
} DiskDriver;

*/

/**
   The blocks indices seen by the read/write functions 
   have to be calculated after the space occupied by the bitmap
*/


// opens the file (creating it if necessary_
// allocates the necessary space on the disk
// calculates how big the bitmap should be
// if the file was new
// compiles a disk header, and fills in the bitmap of appropriate size
// with all 0 (to denote the free space);
void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks){
	//int BLOCK_SIZE = 512;
	int SPACE_ON_DISK = num_blocks*BLOCK_SIZE;
	int fd = open(filename, O_CREAT | O_RDWR, 0666);
	//if(just_created(fd)){
	char buffer[SPACE_ON_DISK];
	memset(buffer, 0, SPACE_ON_DISK);
	int ret = write(fd, buffer, SPACE_ON_DISK);	// scrivo sul file così da allocare la memoria necessaria sul disco
	//}
	void* mymem = mmap(NULL, SPACE_ON_DISK, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	// bitmap must be SPACE_ON_DISK(allocated) / BLOCK_SIZE(512) bits
	int num_bits = num_blocks;
	if (DEBUG) printf("num_bits = %d\nSPACE_ON_DISK = %d\nBLOCK_SIZE = %d\n",num_bits,SPACE_ON_DISK,BLOCK_SIZE);
	int num_chars = num_bits/8;		// number of chars needed by the bitmap
	if (num_chars % 8 != 0) num_chars++;
	if (DEBUG) printf("num_chars = %d\n",num_chars);
	assert(num_chars > 0);
	char entries[num_chars];
	memset(entries, 0, sizeof(char)*num_chars);
	BitMap bm;
	bm.num_bits = num_bits;
	bm.entries = entries;
 
	
	DiskHeader dh;
	dh.num_blocks = num_bits;
	dh.bitmap_blocks = (48+num_chars)/BLOCK_SIZE; 	// 6 int + num_chars char
	if ((48+num_chars)%BLOCK_SIZE != 0) dh.bitmap_blocks++; // round up 
	// set the blocks used by the bitmap and the header as "used (1)" 
	int i;
	for (i=0; i < dh.bitmap_blocks; i++){
		ret = BitMap_set(&bm, i, 1);	// set the bit to 1
		ERROR_HELPER(ret, "Error in set the bit in the bitmap");
	}
	dh.bitmap_entries = num_chars;
	dh.free_blocks = num_bits - dh.bitmap_blocks;
	dh.first_free_block = dh.bitmap_blocks;
	
	// map the bitmap and the header in memory
	DiskHeader* dhm = (DiskHeader*) mymem;
	*dhm = dh;
	char* bmm = (char*) (mymem+sizeof(DiskHeader));
	memcpy(bmm, bm.entries, sizeof(entries));
	
	disk->header = mymem;
	disk->bitmap_data = mymem+sizeof(DiskHeader);
	disk->fd = fd;

	ret = msync(mymem, SPACE_ON_DISK, MS_ASYNC);
	ERROR_HELPER(ret, "Error in msync, disk init");
}

// reads the block in position block_num
// returns -1 if the block is free according to the bitmap
// 0 otherwise
int DiskDriver_readBlock(DiskDriver* disk, void* dest, int block_num){
	BitMap bm;
	bm.num_bits = disk->header->num_blocks;
	bm.entries = disk->bitmap_data;
	if (block_num >= bm.num_bits || block_num < 0)
		return -1; // block required is out of range
	int res = BitMap_get(&bm, block_num, 1);
	// if res != block_num, then the block at index block_num is free
	if (res != block_num){ printf("reas = %d\n",res); return -1;}
	off_t ret = lseek(disk->fd, BLOCK_SIZE*block_num, SEEK_SET);
	ERROR_HELPER(ret, "Error in seek in function readBlock");
	ssize_t read_bytes = read(disk->fd, dest, BLOCK_SIZE);
	/*
	dest = disk->header + BLOCK_SIZE*block_num;
	*/
	// handle all the errors that could occurr
	ERROR_HELPER(read_bytes, "Error in read in function readBlock");
	if (read_bytes < BLOCK_SIZE)
		ERROR_HELPER(-1, "Error: can't read all the bytes of the block");
	if (read_bytes == 0)
		ERROR_HELPER(-1, "Error: end of file");
	return 0;
}
	

// writes a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible, 0 if operation success
int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num){
	BitMap bm;
	bm.num_bits = disk->header->num_blocks;
	bm.entries = disk->bitmap_data;
	int ret = BitMap_get(&bm, block_num, 0);
	// if ret != block_num, the block at index block_num isn't free, so 
	// we can't write in there. Also if ret = -1, there is a fail in 
	// BitMap_get() so we can't write the block as well
	printf("FFB: %d\n", DiskDriver_getFreeBlock(disk, 0));
	printf("dd-ffb:%d, ret: %d, bnum: %d\n\n" ,disk->header->first_free_block, ret, block_num);
	if (ret != block_num || ret < 0) return -1;
	off_t res = lseek(disk->fd, BLOCK_SIZE*block_num, SEEK_SET);
	ERROR_HELPER(res, "Error in seek in function writeBlock");
	ssize_t written_bytes = write(disk->fd, src, BLOCK_SIZE);
	// handle all the errors that could occurr
	ERROR_HELPER(written_bytes, "Error in write in function writeBlock");
	if (written_bytes == 0)
		ERROR_HELPER(-1, "Error: end of file");
	// update the values on disk
	disk->header->free_blocks--;
	// must set the written block as used
	ret = BitMap_set(&bm, block_num, 1);
	if (disk->header->first_free_block == block_num) disk->header->first_free_block = BitMap_get(&bm, 0, 0);  
	
	return 0;
}

// frees a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_freeBlock(DiskDriver* disk, int block_num){
	BitMap bm;
	bm.num_bits = disk->header->num_blocks;
	bm.entries = disk->bitmap_data;
	// conditions if the op isn't possible
	if (block_num >= bm.num_bits || block_num < 0) return -1;
	int ret = BitMap_set(&bm, block_num, 0);
	// if the block freed has index lower than the first free block, update the value
	disk->header->first_free_block = disk->header->first_free_block < ret ? disk->header->first_free_block : ret;
	disk->header->free_blocks--;
	return 0;
}

// returns the first free blockin the disk from position (checking the bitmap)
// NOTE: Can only return a positive integer or launch run-time error
int DiskDriver_getFreeBlock(DiskDriver* disk, int start){
	// recreating the structure BitMap with the data stored in the disk
	BitMap bm;
	bm.num_bits = disk->header->num_blocks;
	bm.entries = disk->bitmap_data;
	// searching for the first free block and then return his index
	int ret = BitMap_get(&bm, start, 0);
	ERROR_HELPER(ret, "Can't find a free block in the bitmap");
	return ret;
}
// writes the data (flushing the mmaps)
// For mappings to files, the msync() function shall ensure that all write
// operations are completed as defined for synchronized I/O data integrity completion
int DiskDriver_flush(DiskDriver* disk){
	void* addr = disk->header;
	int ret = msync(addr, disk->header->num_blocks*BLOCK_SIZE, MS_SYNC);
	return ret;
}
