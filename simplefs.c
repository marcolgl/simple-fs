#include "simplefs.h"
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"

#define FFB_DATA 340	// bytes per i dati nel primo blocco di un file
#define FB_DATA 448		// bytes per i dati in un blocco qualsiasi, diverso dal primo, di un file

#define FDB_DATA 84		// integer per i dati nel primo blocco di una directory
#define DB_DATA 112		// integer per i dati in un blocco qualsiasi, diverso dal primo, di una directory

#define DEBUG 0

#define DEBUG_IN_DIR 0


// initializes a file system on an already made disk
// returns a handle to the top level directory stored in the first block
DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk){
	// SimpleFS fields init
	fs->disk = disk;

	// Creates the top dir '\' and stores it in the first block available 
	int ret, root_dir_block = DiskDriver_getFreeBlock(disk, 0); //disk->header->first_free_block;
	FirstDirectoryBlock* fdb = malloc(sizeof(FirstDirectoryBlock));
	BlockHeader bh;
	bh.previous_block = -1;
	bh.next_block = -1;
	bh.block_in_file = 0; 
	FileControlBlock fcb;
	fcb.directory_block = root_dir_block;
	fcb.block_in_disk = root_dir_block;
	sprintf(fcb.name, "/");
	//printf("\nNAMEDIR: %s\n", fcb.name);
	fcb.size_in_bytes = 0;
	fcb.size_in_blocks = 1;
	fcb.is_dir = 1;

	fdb->header = bh;
	fdb->fcb = fcb;
	fdb->num_entries = 0;
	// fdb->file_blocks uninitialized, cause we have no files in dir yet
	ret =  DiskDriver_writeBlock(disk, fdb, root_dir_block);
	ERROR_HELPER(ret, "Error in fs_init, writeBlock");

	// DirectoryHandle init
	DirectoryHandle* dh = malloc(sizeof(DirectoryHandle));
	dh->sfs = fs;
	dh->dcb = fdb;
	dh->current_block = NULL;
	dh->pos_in_dir = 0;
	dh->pos_in_block = 0;
	dh->block_num = root_dir_block;
	printf("root_dir: %d\n", root_dir_block);
	
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
	int root_dir_block = DiskDriver_getFreeBlock(fs->disk, 0); //dd->header->first_free_block;
	FirstDirectoryBlock* fdb = malloc(sizeof(FirstDirectoryBlock));
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

	fdb->header = bh;
	fdb->fcb = fcb;
	fdb->num_entries = 0;
	// fdb->file_blocks uninitialized, cause we have no files in dir yet
	ret =  DiskDriver_writeBlock(dd, &fdb, root_dir_block);
	ERROR_HELPER(ret, "Error in fs_init, writeBlock");

	// Caches the current_directory_first_block ("\" block) in SimpleFS struct
	fs->current_directory_first_block = fdb;
	if (DEBUG) {BitMap_print(&bm); printf("\nExpected: 111100..\n");}

	return;
}


// reads in the (preallocated) blocks array, the name of all files in a directory 
int SimpleFS_readDir(char** names, DirectoryHandle* d){
	int ret,
		block_index = 0,
	    file_block;
	FirstDirectoryBlock* fdb = d->dcb;	
	int read_entries = 0,
	    num_entries = fdb->num_entries; 
	FirstFileBlock ffb1;
	
	// Read the files' name in the first block of dir
	for (; (block_index < FDB_DATA) && (read_entries < num_entries) ; block_index++){
		if (DEBUG_IN_DIR) printf("\nSearching file in first block...\n");
		file_block = fdb->file_blocks[block_index];
		ret = DiskDriver_readBlock(d->sfs->disk, &ffb1, file_block);
		ERROR_HELPER(ret, "Errore in read block\n");
		if (DEBUG_IN_DIR) printf("filename read: %s\n", ffb1.fcb.name);
		sprintf(names[read_entries], "%s",  ffb1.fcb.name);
		read_entries++;
	}

	DirectoryBlock dblock;		// structure to contain the next directory data block read
	int next_block = fdb->header.next_block; // next dir data block in which search the file
	
	// Read the files' name in the subsequent dir blocks (not in first dir block)
	while (read_entries < num_entries){
		// Read next data block in the structure above allocated
		ret = DiskDriver_readBlock(d->sfs->disk, &dblock, next_block);
		ERROR_HELPER(ret, "Errore in read block, simplefs findfileindir\n");
		for (block_index = 0; (block_index < DB_DATA) && (read_entries < num_entries) ; block_index++){
			if (DEBUG_IN_DIR) printf("\nSearching file in block...\n");
			file_block = dblock.file_blocks[block_index];
			ret = DiskDriver_readBlock(d->sfs->disk, &ffb1, file_block);
			ERROR_HELPER(ret, "Errore in read block, simplefs findfileindir\n");
			sprintf(names[read_entries], "%s",  ffb1.fcb.name);
			read_entries++;
		}
		next_block = dblock.header.next_block;
	}

	return read_entries;	// returns number of file names read
}


// Given a DirectoryHandle, prints recursively his content (including subdirectories content)
void printTreeAux(DiskDriver* disk, FirstDirectoryBlock* fdbInit, int layer){
	int ret,i,
		block_index = 0,
	    file_block;
	FirstDirectoryBlock* fdb = fdbInit;	
	int read_entries = 0,
	    num_entries = fdb->num_entries; 
	FirstFileBlock ffb1;
	
	// Read the files' name in the first block of dir
	for (; (block_index < FDB_DATA) && (read_entries < num_entries) ; block_index++){
		if (DEBUG_IN_DIR) printf("\nSearching file in first block...\n");
		file_block = fdb->file_blocks[block_index];
		ret = DiskDriver_readBlock(disk, &ffb1, file_block);
		ERROR_HELPER(ret, "Errore in read block\n");
		if (DEBUG_IN_DIR) printf("filename read: %s\n", ffb1.fcb.name);
		// Print the file
		for (i=0; i<layer; i++) printf("-");
		printf("%s\n", ffb1.fcb.name);
		// Recursively print if it is a dir
		if (ffb1.fcb.is_dir == 1)
			printTreeAux(disk, (FirstDirectoryBlock*) &ffb1, layer+1);
		read_entries++;
	}

	DirectoryBlock dblock;		// structure to contain the next directory data block read
	int next_block = fdb->header.next_block; // next dir data block in which search the file
	
	// Read the files' name in the subsequent dir blocks (not in first dir block)
	while (read_entries < num_entries){
		// Read next data block in the structure above allocated
		ret = DiskDriver_readBlock(disk, &dblock, next_block);
		ERROR_HELPER(ret, "Errore in read block, simplefs findfileindir\n");
		for (block_index = 0; (block_index < DB_DATA) && (read_entries < num_entries) ; block_index++){
			if (DEBUG_IN_DIR) printf("\nSearching file in block...\n");
			file_block = dblock.file_blocks[block_index];
			ret = DiskDriver_readBlock(disk, &ffb1, file_block);
			ERROR_HELPER(ret, "Errore in read block, simplefs findfileindir\n");
			// Print the file
			for (i=0; i<layer; i++) printf("-");
			printf("%s\n", ffb1.fcb.name);
			// Recursively print if it is a dir
			if (ffb1.fcb.is_dir == 1)
				printTreeAux(disk, (FirstDirectoryBlock*) &ffb1, layer+1);
			read_entries++;
		}
		next_block = dblock.header.next_block;
	}

	return;

}

// AGG: Given a DirectoryHandle, prints recursively his content (including subdirectories content)
void printTree(DirectoryHandle* d){
	printf("%s\n", d->dcb->fcb.name);
	printTreeAux(d->sfs->disk, d->dcb, 1);
}


// seeks for a directory in d. If dirname is equal to ".." it goes one level up
// 0 on success, negative value on error
// it does side effect on the provided handle
int SimpleFS_changeDir(DirectoryHandle* d, char* dirname){
	int ret;
	
	// Save the in memory copy of the directory block, it was only cached til now
	if (d->current_block != NULL){
		ret = DiskDriver_writeBlock(d->sfs->disk, d->current_block, d->block_num);
	}
	else{
		ret = DiskDriver_writeBlock(d->sfs->disk, d->dcb, d->block_num);
	}
	
	// Case: ".." as dirname
	if (memcmp(dirname, "..", strlen(dirname)) == 0){
		if (d->dcb->fcb.directory_block == d->block_num) return 0;	// root directory, can't go up
		// Read parent directory
		FirstDirectoryBlock fdb1;
		ret = DiskDriver_readBlock(d->sfs->disk, &fdb1, d->dcb->fcb.directory_block);
		ERROR_HELPER(ret, "Error in read the block, in change dir");
		
		// if the parent dir has only 1 block
		if (fdb1.header.next_block == -1){
			d->block_num = d->dcb->fcb.directory_block;
			memmove(d->dcb, &fdb1, sizeof(FirstDirectoryBlock));
			d->current_block = NULL;
			d->pos_in_dir = 0;
			d->pos_in_block = fdb1.num_entries; 
		}
		else{
			int next_block_to_read = fdb1.header.next_block, last_block_ind = fdb1.header.next_block;
			DirectoryBlock* db1 = malloc(sizeof(DirectoryBlock)); 
			// if parent dir has more than 1 block i search the last one browsing the list
			while (next_block_to_read != -1){
				ret = DiskDriver_readBlock(d->sfs->disk, db1, db1->header.next_block);
				ERROR_HELPER(ret, "Error in read the block, in change dir");
				if (db1->header.next_block != -1) last_block_ind = db1->header.next_block;
				next_block_to_read = db1->header.next_block;
			}
			SimpleFS_printFirstDirBlock(&fdb1);
			// Modify dir handle
			d->block_num =last_block_ind;
			//*(d->dcb) = fdb1;	// CHECK THIS, MAYBE MEMSET IS NEEDED
			memmove(d->dcb, &fdb1, sizeof(FirstDirectoryBlock));
			if (d->current_block != NULL) free(d->current_block);
			d->current_block = db1;
			d->pos_in_dir = 0;
			d->pos_in_block = (fdb1.num_entries-FDB_DATA)% DB_DATA; 
		}
		return 0;
	}

	// Case: "." as dirname
	if (memcmp(dirname, ".", strlen(dirname)) == 0){printf("bingo2\n"); return 0; }

	// Find directory wanted in the current directory, ret -1 if can't find it
	int dir_block = SimpleFS_findDirInDir(d, dirname);
	if (DEBUG) printf("curr_block = %d, dir_block = %d\n",d->block_num, dir_block);
	if (dir_block < 0) return -1;	// Error in find dir
	//ERROR_HELPER(dir_block, "Error, can't find dir in dir");
	if (DEBUG) printf("curr_block = %d, dir_block = %d\n",d->block_num, dir_block);

	// Check dir found is a directory and not a file
	FirstDirectoryBlock fdb;
	ret = DiskDriver_readBlock(d->sfs->disk, &fdb, dir_block);
	if (ret < 0) return -1;		// Error in read block
	if (fdb.fcb.is_dir == 0) return -1; 	// Error, file found, not a directory 	

	// Modify dir handle
	d->block_num = dir_block;
	//(d->dcb) = fdb;	// CHECK THIS, MAYBE MEMSET IS NEEDED
	memmove(d->dcb, &fdb, sizeof(FirstDirectoryBlock));
	d->current_block = NULL;
	d->pos_in_dir = 0;
	d->pos_in_block = 0; 
	return 0;
	
}




// returns the block in disk at which is stored the first file block 
// returns -1 if there is no such file in directory
int SimpleFS_findFileInDir(DirectoryHandle* d, const char* filename){
	int ret,
	    block_index = 0,
	    file_block;
	FirstDirectoryBlock* fdb = d->dcb;	
	int read_entries = 0,
	    num_entries = fdb->num_entries; 
	FirstFileBlock ffb1;
	
	// Search the file in the first directory block
	for (; (block_index < FDB_DATA) && (read_entries < num_entries) ; block_index++){
		if (DEBUG_IN_DIR) printf("Searching file in first block...\n");
		file_block = fdb->file_blocks[block_index];
		ret = DiskDriver_readBlock(d->sfs->disk, &ffb1, file_block);
		ERROR_HELPER(ret, "Errore in read block\n");
		read_entries++;
		if (ffb1.fcb.is_dir == 0 && memcmp(ffb1.fcb.name, filename, strlen(filename)) == 0){
			return file_block;	// file found in directory entries
		}
	}

	DirectoryBlock dblock;		// structure to contain the next directory data block read
	int next_block = fdb->header.next_block; // next dir data block in which search the file
	
	// Search the file in subsequent directory blocks (not it first dir block)
	while (read_entries < num_entries){
		// Read next data block in the structure above allocated
		ret = DiskDriver_readBlock(d->sfs->disk, &dblock, next_block);
		ERROR_HELPER(ret, "Errore in read block, simplefs findfileindir\n");
		
		for (block_index = 0; (block_index < DB_DATA) && (read_entries < num_entries) ; block_index++){
			if (DEBUG_IN_DIR) printf("\nSearching file in block...\n");
			file_block = dblock.file_blocks[block_index];
			ret = DiskDriver_readBlock(d->sfs->disk, &ffb1, file_block);
			ERROR_HELPER(ret, "Errore in read block, simplefs findfileindir\n");
			read_entries++;
			if (ffb1.fcb.is_dir == 0 && memcmp(ffb1.fcb.name, filename, strlen(filename) == 0))
				return file_block;	// file found in directory entries
		}
		next_block = dblock.header.next_block;
	}
	if (DEBUG_IN_DIR) printf("Not found\n");
	return -1;	// file not found
}


// returns the block in disk at which is stored the first dir block 
// returns -1 if there is no such dir in directory
int SimpleFS_findDirInDir(DirectoryHandle* d, const char* dirname){
	int ret,
	    block_index = 0,
	    file_block;
	FirstDirectoryBlock* fdb = d->dcb;	
	int read_entries = 0,
	    num_entries = fdb->num_entries; 
	FirstFileBlock ffb1;
	
	// Search the file in the first directory block
	for (; (block_index < FDB_DATA) && (read_entries < num_entries) ; block_index++){
		if (DEBUG_IN_DIR) printf("Searching file in first block...\n");
		file_block = fdb->file_blocks[block_index];
		ret = DiskDriver_readBlock(d->sfs->disk, &ffb1, file_block);
		ERROR_HELPER(ret, "Errore in read block\n");
		read_entries++;
		if (ffb1.fcb.is_dir == 1 && memcmp(ffb1.fcb.name, dirname, strlen(dirname)) == 0)
			return file_block;	// file found in directory entries
	}

	DirectoryBlock dblock;		// structure to contain the next directory data block read
	int next_block = fdb->header.next_block; // next dir data block in which search the file
	
	// Search the file in subsequent directory blocks (not it first dir block)
	while (read_entries < num_entries){
		// Read next data block in the structure above allocated
		ret = DiskDriver_readBlock(d->sfs->disk, &dblock, next_block);
		ERROR_HELPER(ret, "Errore in read block, simplefs findfileindir\n");
		
		for (block_index = 0; (block_index < DB_DATA) && (read_entries < num_entries) ; block_index++){
			if (DEBUG_IN_DIR) printf("\nSearching file in block...\n");
			file_block = dblock.file_blocks[block_index];
			ret = DiskDriver_readBlock(d->sfs->disk, &ffb1, file_block);
			ERROR_HELPER(ret, "Errore in read block, simplefs findfileindir\n");
			read_entries++;
			if (ffb1.fcb.is_dir == 1 && memcmp(ffb1.fcb.name, dirname, strlen(dirname) == 0))
				return file_block;	// file found in directory entries
		}
		next_block = dblock.header.next_block;
	}
	if (DEBUG_IN_DIR) printf("Not found\n");
	return -1;	// file not found
}



// opens a file in the  directory d. The file should be exisiting
// AGG: returns NULL if file doesn't exist in the directory
FileHandle* SimpleFS_openFile(DirectoryHandle* d, const char* filename){
	int ret;
	int file_block = SimpleFS_findFileInDir(d, filename);
	if (file_block < 0) return NULL; // file not found in dir
	
	// Allocate a structure to save first file block in main memory
	FirstFileBlock* ffb = malloc(sizeof(FirstFileBlock));
	ret = DiskDriver_readBlock(d->sfs->disk, ffb, file_block);
	ERROR_HELPER(ret, "Error in read block, in open file\n");

	// Allocate the file handler and return it
	FileHandle* fh = malloc(sizeof(FileHandle));
	fh->sfs = d->sfs;
	fh->fcb = ffb;				
	fh->current_block = NULL;
	fh->block_num = file_block;
	fh->pos_in_file = 0;

	return fh;
}

// closes a file handle (destroyes it)
int SimpleFS_close(FileHandle* f){
	free(f->fcb);
	if (f->current_block != NULL) free(f->current_block);
	free(f);
	return 0;
}

int min(int a, int b){
	if (a <= b) return a;
	return b;
}

// writes in the file, at current position for size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes written
int SimpleFS_write(FileHandle* f, void* data, int size){
	int ret, written_bytes = 0;
	int space_av_in_block;
	int writing_size;
	// Case we are in first file block
	if (f->current_block == NULL){
		space_av_in_block = FFB_DATA - f->pos_in_file;
		writing_size = min(space_av_in_block, size);
		memcpy(f->fcb->data + f->pos_in_file, data, writing_size);
		written_bytes += writing_size;
		f->pos_in_file += writing_size;
		f->fcb->fcb.size_in_bytes += writing_size;
	}
	// Case we are in a generic block (not in the first)
	else{	
		space_av_in_block = FB_DATA -f->pos_in_file;
		writing_size = min(space_av_in_block, size);
		memcpy(f->current_block->data + f->pos_in_file, data, writing_size);
		written_bytes += writing_size;
		f->pos_in_file += writing_size;
		f->fcb->fcb.size_in_bytes += writing_size;
	}

	// Save the block on which i have just written on disk
	if (f->current_block == NULL){
		ret = DiskDriver_writeBlock(f->sfs->disk, f->fcb, f->block_num);
		ERROR_HELPER(ret, "Error in write block in write op");
	}
	else{
		ret = DiskDriver_writeBlock(f->sfs->disk, f->current_block, f->block_num);
		ERROR_HELPER(ret, "Error in write block in write op");
	}

	while (written_bytes < size){
		// if i am here i need to allocate another block to the file to complete the write
		int new_block = DiskDriver_getFreeBlock(f->sfs->disk, 0);
		ERROR_HELPER(new_block, "Error in get free block in write");
		
		// Allocate new block
		FileBlock* fb = malloc(sizeof(FileBlock));
		BlockHeader bh;
		bh.previous_block = f->block_num;
		bh.next_block = -1;
		if (f->current_block == NULL) bh.block_in_file = 1;
		else bh.block_in_file = f->current_block->header.block_in_file +1;
		fb->header = bh;
		// Update filehandle with current block
		f->current_block = fb;
		f->block_num = new_block;
		f->pos_in_file = 0;

		// Write again
		space_av_in_block = FB_DATA - f->pos_in_file;
		writing_size = min(space_av_in_block, size-written_bytes);
		memcpy(f->current_block->data + f->pos_in_file, data, writing_size);
		written_bytes += writing_size;
		f->pos_in_file += writing_size;
		f->fcb->fcb.size_in_bytes += writing_size;

		// Save the block on which i have just written on disk
		ret = DiskDriver_writeBlock(f->sfs->disk, f->current_block, f->block_num);
		ERROR_HELPER(ret, "Error in write block in write op");
	}

	return 0;
}

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
FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename){
	int ret,designed_block;
	
	// Check file isn't already in dir
	ret = SimpleFS_findFileInDir(d, filename);
	if (ret >= 0) return NULL; 		// file already in directory

	// Check there is a block available on disk
	designed_block = DiskDriver_getFreeBlock(d->sfs->disk, 0);
	if (designed_block == -1) return NULL;		// no free blocks
	

	// Create new file block and allocate in disk and cache
	ret = DiskDriver_setBlock(d->sfs->disk, designed_block, 1);
	ERROR_HELPER(ret, "Error in set block, in create file");

	FirstFileBlock* ffb = malloc(sizeof(FirstFileBlock));
	BlockHeader bh;
	bh.previous_block = -1;
	bh.next_block = -1;
	bh.block_in_file = 0;
	FileControlBlock fcb;
	fcb.directory_block = d->dcb->fcb.block_in_disk;
	fcb.block_in_disk = designed_block;
	sprintf(fcb.name, "%s", filename);
	fcb.size_in_bytes = 0;
	fcb.size_in_blocks = 0;
	fcb.is_dir = 0;

	ffb->header = bh;
	ffb->fcb = fcb;
	ret = DiskDriver_writeBlock(d->sfs->disk, ffb, designed_block);
	ERROR_HELPER(ret, "Can't write on disk, in file creation");

	// Add file on directory data	
		// writing in data directory block
		if (d->current_block != NULL){
			d->current_block->file_blocks[d->pos_in_block] = designed_block;
			d->dcb->num_entries++;
			d->pos_in_block++;
			// if reach last byte of data, i allocate another block
			if (d->pos_in_block == FB_DATA){
				// write the in memory current block to disk
				ret = DiskDriver_writeBlock(d->sfs->disk, d->current_block, d->block_num);
				ERROR_HELPER(ret, "Error in write block, in create file");
				// allocation of new block on disk and in cache
				d->pos_in_block = 0;
				d->current_block->header.block_in_file++;
				d->current_block->header.next_block = -1;
				d->current_block->header.previous_block = d->block_num;
				ret = DiskDriver_getFreeBlock(d->sfs->disk, 0);
				ERROR_HELPER(ret, "Can't find a free block, in create file");
				d->block_num = DiskDriver_writeBlock(d->sfs->disk, d->current_block, ret);

			}		
		}
		// writing in first directory block data
		if (d->current_block == NULL){
			d->dcb->file_blocks[d->pos_in_block] = designed_block;
			d->dcb->num_entries++;
			d->pos_in_block++;
			// if reach last byte of data, i allocate another block
			if (d->pos_in_block == FDB_DATA){
				// write the in memory current block to disk
				ret = DiskDriver_writeBlock(d->sfs->disk, d->dcb, d->block_num);
				ERROR_HELPER(ret, "Error in write block, in create file");
				// allocation of new block on disk and in cache
				d->current_block = malloc(sizeof(DirectoryBlock));
				d->pos_in_block = 0;
				d->current_block->header.block_in_file++;
				d->current_block->header.next_block = -1;
				d->current_block->header.previous_block = d->block_num;
				ret = DiskDriver_getFreeBlock(d->sfs->disk, 0);
				ERROR_HELPER(ret, "Can't find a free block, in create file");
				d->block_num = DiskDriver_writeBlock(d->sfs->disk, d->current_block, ret);

			}
		}

	// Build FileHandle
	FileHandle* fhandle = malloc(sizeof(FileHandle));
	fhandle->sfs = d->sfs;
	fhandle->fcb = ffb;
	//fhandle->directory = d->dcb;
	fhandle->current_block = NULL;
	fhandle->block_num = designed_block;
	fhandle->pos_in_file = 0;
	
	return fhandle;
}


// creates a new directory in the current one (NO: stored in fs->current_directory_block)
// 0 on success
// -1 on error
int SimpleFS_mkDir(DirectoryHandle* d, char* dirname){
	int ret,designed_block;
	
	// Check new dir isn't already in dir
	ret = SimpleFS_findDirInDir(d, dirname);
	if (DEBUG) printf("Trovato: %d\n", ret);
	if (ret >= 0) return -1; 		// file already in directory

	// Check there is a block available on disk
	designed_block = DiskDriver_getFreeBlock(d->sfs->disk, 0);
	if (designed_block == -1) return -1;		// no free blocks

	// Create new dir block and allocate in disk and cache
	ret = DiskDriver_setBlock(d->sfs->disk, designed_block, 1);
	ERROR_HELPER(ret, "Error in set block, in create file");

	FirstDirectoryBlock* fdb = malloc(sizeof(FirstDirectoryBlock));
	BlockHeader bh;
	bh.previous_block = -1;
	bh.next_block = -1;
	bh.block_in_file = 0;
	FileControlBlock fcb;
	fcb.directory_block = d->dcb->fcb.block_in_disk;
	fcb.block_in_disk = designed_block;
	sprintf(fcb.name, "%s", dirname);
	fcb.size_in_bytes = 0;
	fcb.size_in_blocks = 0;
	fcb.is_dir = 1;

	fdb->header = bh;
	fdb->fcb = fcb;
	ret = DiskDriver_writeBlock(d->sfs->disk, fdb, designed_block);
	ERROR_HELPER(ret, "Can't write on disk, in file creation");

	// Add new dir on directory data	
		// writing in data directory block
		if (d->current_block != NULL){
			d->current_block->file_blocks[d->pos_in_block] = designed_block;
			d->dcb->num_entries++;
			d->pos_in_block++;
			// if reach last byte of data, i allocate another block
			if (d->pos_in_block == FB_DATA){
				// write the in memory current block to disk
				ret = DiskDriver_writeBlock(d->sfs->disk, d->current_block, d->block_num);
				ERROR_HELPER(ret, "Error in write block, in create file");
				// allocation of new block on disk and in cache
				d->pos_in_block = 0;
				d->current_block->header.block_in_file++;
				d->current_block->header.next_block = -1;
				d->current_block->header.previous_block = d->block_num;
				ret = DiskDriver_getFreeBlock(d->sfs->disk, 0);
				ERROR_HELPER(ret, "Can't find a free block, in create file");
				d->block_num = DiskDriver_writeBlock(d->sfs->disk, d->current_block, ret);

			}		
		}
		// writing in first directory block data
		if (d->current_block == NULL){
			d->dcb->file_blocks[d->pos_in_block] = designed_block;
			d->dcb->num_entries++;
			d->pos_in_block++;
			// if reach last byte of data, i allocate another block
			if (d->pos_in_block == FDB_DATA){
				// write the in memory current block to disk
				ret = DiskDriver_writeBlock(d->sfs->disk, d->dcb, d->block_num);
				ERROR_HELPER(ret, "Error in write block, in create file");
				// allocation of new block on disk and in cache
				d->current_block = malloc(sizeof(DirectoryBlock));
				d->pos_in_block = 0;
				d->current_block->header.block_in_file++;
				d->current_block->header.next_block = -1;
				d->current_block->header.previous_block = d->block_num;
				ret = DiskDriver_getFreeBlock(d->sfs->disk, 0);
				ERROR_HELPER(ret, "Can't find a free block, in create file");
				d->block_num = DiskDriver_writeBlock(d->sfs->disk, d->current_block, ret);

			}
		}

	return 0;
}


// Prints a DirectoryHandle 
void SimpleFS_printDirHandle(DirectoryHandle* d){
	printf("DirectoryHandle representation:\n");
	//print sfs
	//print dcb
	//print curr_block
	printf("\tblock_num: %d\n", d->block_num);
	printf("\tpos_in_dir: %d\n", d->pos_in_dir);
	printf("\tpos_in_block: %d\n", d->pos_in_block);
}

// Prints a FileHandle
void SimpleFS_printFileHandle(FileHandle* f){
	printf("FileHandle representation:\n");
	//print sfs
	//print fcb
	//print directory
	//print current_block
	printf("\tblock_num: %d\n", f->block_num);
	printf("\tpos_in_file: %d\n", f->pos_in_file);
}

void SimpleFS_printFirstDirBlock(FirstDirectoryBlock* fdb){
	printf("FIRST DIR BLOCK:\n");
	printf("-fcb.name: %s\n", fdb->fcb.name);
	printf("-num_entries: %d\n", fdb->num_entries);
	printf("-data: ");
	int i = 0;
	int datasize = min(fdb->num_entries, FDB_DATA);
	for (; i< datasize; i++){
		printf("|%d", fdb->file_blocks[i]);
	}
	printf("|\n");
}


void SimpleFS_printFirstFileBlock(FirstFileBlock* ffb){
	printf("FIRST FILE BLOCK:\n");
	printf("-fcb.name: %s\n", ffb->fcb.name);
	printf("-data: ");
	int i = 0;
	int datasize = min(ffb->fcb.size_in_bytes, FFB_DATA);
	for (; i< datasize; i++){
		printf("%c", ffb->data[i]);
	}
	printf("|\n");
}

void SimpleFS_printFileBlock(FileBlock* fb){
	printf("FILE BLOCK:\n");
	printf("-data: ");
	int i = 0;
	int datasize = FB_DATA;
	for (; i< datasize; i++){
		printf("%c", fb->data[i]);
	}
	printf("|\n"); 
}