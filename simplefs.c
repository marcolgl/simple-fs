#include "simplefs.h"
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"

#define FFB_DATA 352	// bytes per i dati nel primo blocco di un file
#define FB_DATA 500		// bytes per i dati in un blocco qualsiasi, diverso dal primo, di un file

#define DEBUG 0



// initializes a file system on an already made disk
// returns a handle to the top level directory stored in the first block
DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk){
	// SimpleFS fields init
	fs->disk = disk;

	// Creates the top dir '\' and stores it in the first block available 
	int ret, root_dir_block = disk->header->first_free_block;
	FirstDirectoryBlock fdb;
	BlockHeader bh;
	bh.previous_block = -1;
	bh.next_block = -1;
	bh.block_in_file = 0; 
	FileControlBlock fcb;
	fcb.directory_block = root_dir_block;
	fcb.block_in_disk = root_dir_block;
	sprintf(fcb.name, "/");
	fcb.size_in_bytes = 0;
	fcb.size_in_blocks = 1;
	fcb.is_dir = 1;

	fdb.header = bh;
	fdb.fcb = fcb;
	fdb.num_entries = 0;
	// fdb->file_blocks uninitialized, cause we have no files in dir yet
	ret =  DiskDriver_writeBlock(disk, &fdb, root_dir_block);
	ERROR_HELPER(ret, "Error in fs_init, writeBlock");

	// DirectoryHandle init
	DirectoryHandle* dh = malloc(sizeof(DirectoryHandle));
	dh->sfs = fs;
	/////// MMAP ALL FS
	dh->dcb = disk->header + BLOCK_SIZE*root_dir_block;
	dh->current_block = &(dh->dcb->header);
	///////
	dh->pos_in_dir = 0;
	dh->pos_in_block = 0;
	
	// returns the root directory handle
	return dh;
}


// creates the inital structures, the top level directory
// has name "/" and its control block is in the first position
// it also clears the bitmap of occupied blocks on the disk
// the current_directory_block is cached in the SimpleFS struct
// and set to the top level directory
void SimpleFS_format(SimpleFS* fs){
	
	// Set the DiskHeader init values 
	DiskDriver* dd = fs->disk; 
	dd->header->free_blocks = dd->header->num_blocks - dd->header->bitmap_blocks;
	dd->header->first_free_block = dd->header->bitmap_blocks;

	// Clear the BitMap of allocated blocks on disk
	BitMap bm;
	bm.num_bits = dd->header->num_blocks;
	bm.entries = dd->bitmap_data;
	int ret, allocated_block = 0;
	if (DEBUG){ BitMap_print(&bm); printf("\nExpected: 111100..\n");}
	while (allocated_block != -1){
		allocated_block = BitMap_get(&bm, dd->header->first_free_block, 1);  // finds the first allocated block not used to store the metadata (bitmap, diskheader, etc)
		ret = BitMap_set(&bm, allocated_block, 0);
		ERROR_HELPER(ret, "Error in reset the bitmap, in SimpleFS_format");
	}
	if (DEBUG){ BitMap_print(&bm); printf("\nExpected: 111000..\n");}


	// Creates the top dir '\' and stores it in the first block available 
	int root_dir_block = dd->header->first_free_block;
	FirstDirectoryBlock fdb;
	BlockHeader bh;
	bh.previous_block = -1;
	bh.next_block = -1;
	bh.block_in_file = 0; 
	FileControlBlock fcb;
	fcb.directory_block = root_dir_block;
	fcb.block_in_disk = root_dir_block;
	sprintf(fcb.name, "/");
	fcb.size_in_bytes = 0;
	fcb.size_in_blocks = 1;
	fcb.is_dir = 1;

	fdb.header = bh;
	fdb.fcb = fcb;
	fdb.num_entries = 0;
	// fdb->file_blocks uninitialized, cause we have no files in dir yet
	ret =  DiskDriver_writeBlock(dd, &fdb, root_dir_block);
	ERROR_HELPER(ret, "Error in fs_init, writeBlock");

	// Caches the current_directory_block ("\" block) in SimpleFS struct
	// WITH ALL FS MMAPPED
	fs->current_directory_block = dd->header + BLOCK_SIZE*root_dir_block;
	if (DEBUG) {BitMap_print(&bm); printf("\nExpected: 111100..\n");}

	return;
}



// returns the block in disk at which is stored the first file block 
// returns -1 if there is no such file in directory
/*int SimpleFS_findFileInDir(DirectoryHandle* d, const char* filename){
	int ret, found = 0, block_index = 0;
	FirstFileBlock ffb1;
	for (; block_index < dbc1->num_entries; block_index++){
		int block_num = dcb1->file_blocks[block_index];
		ret = DiskDriver_readBlock(d->sfs->disk, &ffb1, block_num);
		ERROR_HELPER(ret, "Errore in read block\n");
		if (ffb1.fcb.is_dir == 0 && memcmp(ffb1.fcb.name, filename, strlen(filename) == 0){
			found=1;	// file found
			break;
		}
	}
	if (found == 0) return -1; 	// file not found in the directory
	return block_num;
}
*/
	


// opens a file in the  directory d. The file should be exisiting
// AGG: aggiunge il file nella lista dei file aperti nel sistema se non
//		è presente, altrimenti incrementa il numero dei processi da cui 
//		il file è stato aperto
// AGG: returns NULL if file doesn't exist in the directory
/*FileHandle* SimpleFS_openFile(DirectoryHandle* d, const char* filename){	//TO CHECK
	FirstDirectoryBlock* dcb1 = d->dcb;
	int ret = SimpleFS_findFileInDir(d, filename);
	if (ret < 0) return NULL;
	FileHandle* fh = malloc(sizeof(FileHandle));
	fh->sfs = d->sfs;
	fh->fcb = d->sfs->disk->header + BLOCK_SIZE*ret;		// TO CHECK
	fh->directory = dcb1;
	fh->current_block = fh->fcb;
	fh->pos_in_file = 0;
	fh->curr_block = ffb1.fcb.block_in_disk;
	return fh;
}
*/

// returns the number of bytes read (moving the current pointer to pos)*/
// returns pos on success
// -1 on error (file too short)
/*int SimpleFS_read(FileHandle* f, void* data, int size){
	int read_bytes = 0;
	int ret;
	int temp[BLOCK_SIZE];
	int left = 0;
	if (size > f->fcb->size_in_bytes) return -1; 	// not enough bytes written
	// posizione del cursore all'interno del blocco, relativa
	int relative_in_block_position = f->pos_in_file;
	while (read_bytes < size){
		ret = DiskDriver_readBlock(f->sfs->disk, temp, f->curr_block);
		ERROR_HELPER(ret, "Error in readBlock in sfs_read");
		// in case we are reading the last block partially
		if (size-read_bytes+relative_in_block_postition < BLOCK_SIZE){
			memcpy(data+read_bytes, temp+relative_in_block_position, size-read_bytes); 
			left = size-read_bytes;
		}
		// in case of reading the first k blocks (the first included)
		else
			memcpy(data+read_bytes, temp+relative_in_block_position, BLOCK_SIZE-relative_in_block_position);
		relative_in_block_position = 0;
		if (f->current_block->next_block = -1) return -1; // file too short
		f->curr_block = f->current_block->next_block;
	}
	f->pos_in_file = left;
	return 0;
}


*/
	
// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
/*FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename){
	int ret,designed_block;
	designed_block = DiskDriver_getFreeBlock(d->sfs->disk, 0);
	if (ret == -1) return NULL;		// no free blocks
	ret = SimpleFS_findFileInDir(d, filename);
	if (ret >= 0) return NULL; 		// file already in directory
	BlockHeader* bh;
	bh->previous_block = -1;
	bh->next_block = -1;
	bh->block_in_file = 0;
	FileControlBlock* fcb;
	fcb->directory_block = d->directory;
	fcb->block_in_disk = designed_block;
	sprintf(fcb->name, filename);
	fcb->size_in_bytes = 0;
	fcb->size_in_blocks = 1;
	fcb->is_dir = 0;
	FirstFileBlock* ffb;
	ffb->header = bh;
	ffb->fcb = fcb;
	ret = DiskDriver_writeBlock(d->sfs->disk, &ffb, designed_block);
	FileHandle* fh = malloc(sizeof(FileHandle));
	fh->sfs = d->sfs;
	fh->fcb = d->sfs->disk->header + BLOCK_SIZE*designed_block;
	fh->directory = d->directory;
	fh->current_block = fh->fcb;
	fh->pos_in_file = 0;
	fh->curr_block = designed_block;
	return fh;
}
*/
	
	

	
	
/*
typedef struct {
  SimpleFS* sfs;                   // pointer to memory file system structure
  FirstFileBlock* fcb;             // pointer to the first block of the file(read it)
  FirstDirectoryBlock* directory;  // pointer to the directory where the file is stored
  BlockHeader* current_block;      // current block in the file
  int pos_in_file;                 // position of the cursor (AGG:relative)
  int curr_block;				   // AGG: current block on disk
} FileHandle;

typedef struct {
  SimpleFS* sfs;                   // pointer to memory file system structure
  FirstDirectoryBlock* dcb;        // pointer to the first block of the directory(read it)
  FirstDirectoryBlock* directory;  // pointer to the parent directory (null if top level)
  BlockHeader* current_block;      // current block in the directory
  int pos_in_dir;                  // absolute position of the cursor in the directory
  int pos_in_block;                // relative position of the cursor in the block
} DirectoryHandle;
*/	
// this is the first physical block of a directory
/*typedef struct {
  BlockHeader header;
  FileControlBlock fcb;
  int num_entries;
  int file_blocks[ (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
		    -sizeof(int))/sizeof(int) ];
} FirstDirectoryBlock;
*/
// closes a file handle (destroyes it)
// AGG: decrementa il numero dei processi che hanno aperto il file, se 
//		uguale a 0, elimina la entry
//int SimpleFS_close(FileHandle* f);
