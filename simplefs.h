#pragma once
#include "bitmap.h"
#include "disk_driver.h"

/*these are structures stored on disk*/

// header, occupies the first portion of each block in the disk
// represents a chained list of blocks
typedef struct {
  int previous_block; // chained list (previous block)
  int next_block;     // chained list (next_block) 		//AGG: -1 se non vi è un blocco successivo
  int block_in_file; // position in the file, if 0 we have a file control block
} BlockHeader;


// this is in the first block of a chain, after the header
typedef struct {
  int directory_block; // first block of the parent directory
  int block_in_disk;   // repeated position of the block on the disk
  char name[128];
  int  size_in_bytes; // AGG: size of data excluding metadata
  int size_in_blocks;
  int is_dir;          // 0 for file, 1 for dir
} FileControlBlock;

// this is the first physical block of a file
// it has a header
// an FCB storing file infos
// and can contain some data

/******************* stuff on disk BEGIN *******************/
typedef struct {
  BlockHeader header;
  FileControlBlock fcb;
  char data[BLOCK_SIZE-sizeof(FileControlBlock) - sizeof(BlockHeader)] ;
} FirstFileBlock;

// this is one of the next physical blocks of a file
typedef struct {
  BlockHeader header;
  char  data[BLOCK_SIZE-sizeof(BlockHeader)];
} FileBlock;

// this is the first physical block of a directory
typedef struct {
  BlockHeader header;
  FileControlBlock fcb;
  int num_entries;
  int file_blocks[ (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
		    -sizeof(int))/sizeof(int) ];
} FirstDirectoryBlock;

// i choose to mantain only the FirstFileBlocks in the array file_blocks of a dir
// cause i can easily found the other blocks recursively scanning the list in the headers


// this is remainder block of a directory
typedef struct {
  BlockHeader header;
  int file_blocks[ (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int) ];
} DirectoryBlock;
/******************* stuff on disk END *******************/




  
typedef struct {
  DiskDriver* disk;
  FirstDirectoryBlock* current_directory_first_block;
  // add more fields if needed
} SimpleFS;

// this is a file handle, used to refer to open files
typedef struct {
  SimpleFS* sfs;                   // pointer to memory file system structure
  FirstFileBlock* fcb;             // pointer to the first block of the file(read it)
  //FirstDirectoryBlock* directory;  // pointer to the directory where the file is stored
  FileBlock* current_block;        // current block in the file, null if current is FirstFileBlock
  int block_num;                   // AGG: block num on disk
  int pos_in_file;                 // position of the cursor
} FileHandle;

typedef struct {
  SimpleFS* sfs;                   // pointer to memory file system structure
  FirstDirectoryBlock* dcb;        // pointer to the first block of the directory(read it)
  //FirstDirectoryBlock* directory;  // pointer to the parent directory (null if top level)
  DirectoryBlock* current_block;   // current block in the directory, null if curr_position is FirstDirBlock
  int block_num;                   // AGG: block num on disk
  int pos_in_dir;    //remove?              // absolute position of the cursor in the directory
  int pos_in_block;                // relative position of the cursor in the block
} DirectoryHandle;


// AGG:
int SimpleFS_findFileInDir(DirectoryHandle* d, const char* filename);
// AGG:
void SimpleFS_printDirHandle(DirectoryHandle* d);
// AGG:
void SimpleFS_printFileHandle(FileHandle* f);
// AGG: Given a DirectoryHandle, prints recursively his content (including subdirectories content)
void printTree(DirectoryHandle* d);
// AGG: 
int SimpleFS_findDirInDir(DirectoryHandle* d, const char* dirname);
// AGG:
void SimpleFS_printFirstDirBlock(FirstDirectoryBlock* fdb);
// AGG:
void SimpleFS_printFirstFileBlock(FirstFileBlock* ffb);
// AGG:
void SimpleFS_printFileBlock(FileBlock* fb);




// initializes a file system on an already made disk
// returns a handle to the top level directory stored in the first block
DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk);

// creates the inital structures, the top level directory
// has name "/" and its control block is in the first position
// it also clears the bitmap of occupied blocks on the disk
// the current_directory_block is cached in the SimpleFS struct
// and set to the top level directory
void SimpleFS_format(SimpleFS* fs);

// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename);

// reads in the (preallocated) blocks array, the name of all files in a directory 
int SimpleFS_readDir(char** names, DirectoryHandle* d);


// opens a file in the  directory d. The file should be exisiting
FileHandle* SimpleFS_openFile(DirectoryHandle* d, const char* filename);


// closes a file handle (destroyes it)
int SimpleFS_close(FileHandle* f);

// writes in the file, at current position for size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes written
int SimpleFS_write(FileHandle* f, void* data, int size);

// read from current position size bytes and store them in data, an already allocated array,
// returns the number of bytes read
int SimpleFS_read(FileHandle* f, void* data, int size);

// returns pos on success
// -1 on error (file too short)
int SimpleFS_seek(FileHandle* f, int pos);

// seeks for a directory in d. If dirname is equal to ".." it goes one level up
// 0 on success, negative value on error
// it does side effect on the provided handle
int SimpleFS_changeDir(DirectoryHandle* d, char* dirname);

// creates a new directory in the current one (stored in fs->current_directory_block)
// 0 on success
// -1 on error
int SimpleFS_mkDir(DirectoryHandle* d, char* dirname);

// Modified: first param changed to DirectoryHandle*
// removes the file in the directory represented by directory handle
// returns -1 on failure 0 on success
// if a directory, it removes recursively all contained files
int SimpleFS_remove(DirectoryHandle* d, char* filename);


/*
 * STRUTTURE AGGIUNTE ALLE SPECIFICHE
 */
 

//AGG: entry dell'open-files table
typedef struct{
	const char filename[128];
	int num_opened;
	//ProcList pl;
	struct OpenFileEntry* next;	// puntatore alla prossima entry
} OpenFileEntry;


//AGG:	open-files table
typedef struct{
	OpenFileEntry* first;	// puntatore alla prima entry del files-table 
} OpenFileTable;



      // this function move the last entry to the position of the entry deleted and 
// if no more entries are on the last block, deallocate the last block from disk
// it also update the num_entries on first dir block(dir control block)
// PARAMS : d - DirectoryHandle
//			entry_pos - entry position in block of the file to remove ( we need to remove the entry at this position) 
// 			block_inf_entry_to_remove - number of the block in file of the entry to remove
// 			file_block - block_num in disk of the file to remove

/*int remove_file_from_dir(DirectoryHandle* d, int entry_pos, int block_inf_entry_to_remove, int file_block){
	int ret, counter;
	DirectoryBlock db_l; 
	DirectoryBlock db_r;

	// find the block in file position of last entry (0 if in control block)
	int last_entry_block;
	if (d->dcb->num_entries <=  FFB_DATA){
		last_entry_block = 0;
	}
	else {
		last_entry_block = (d->dcb->num_entries - FFB_DATA)/DB_DATA +1;
		if ((d->dcb->num_entries - FFB_DATA) % DB_DATA == 0) last_entry_block--;
	}

	// if last block is first dir block
	if (last_entry_block == 0){
		d->dcb->file_blocks[entry_pos] = d->dcb->file_blocks[d->dcb->num_entries-1];
	}

	// find last_pos, position in block of last entry
	int last_pos;
	if ( FFB_DATA +FB_DATA*last_entry_block- d->dcb->num_entries == 0)
		last_pos = FB_DATA -1;
	else if ( FFB_DATA +FB_DATA*last_entry_block- d->dcb->num_entries == FB_DATA)
		last_pos = 0;
	else
		last_pos = FFB_DATA + FB_DATA*last_entry_block - d->dcb->num_entries;
	//


	int next_block = d->dcb->header.next_block;
	// if last block is not first dir block and the file to remove entry is stored in the first dir block
	if (last_entry_block != 0 && block_inf_entry_to_remove == 0){

		// read the last block in memory
		counter = 0;
		while (counter < last_entry_block){
			ret = DiskDriver_readBlock(d->sfs->disk, &db_l, next_block);
			ERROR_HELPER(ret, "Error in read, in remove_file_from_dir\n");
			counter++;
			next_block = db_l.header.next_block;
		}

		d->dcb->file_blocks[entry_pos] = db_l.file_blocks[last_pos];
		
		// case in which we need to remove the blank block
		if (last_pos == 0){
			
		}
	}	


	


	// Decrease num_entries in directory
	d->dcb->num_entries--;
}
*/

// removes the file in the directory represented by directory handle
// returns -1 on failure 0 on success
// if a directory, it removes recursively all contained files
/*int SimpleFS_remove(DirectoryHandle* d, char* filename){
	int ret;
	int read_entries = 0;
	int file_block;		// here we store the block_num of the file scanned
	FirstFileBlock ffb;

	// recursively remove all files
	if (memcmp(filename, "*", strlen(filename)) == 0){
		printf("TODO : Remove all files\n");
	}


	// scanning the first directory block
	while (read_entries < d->dcb->num_entries && read_entries < FDB_DATA){
		file_block = d->dcb->file_blocks[read_entries];
		ret = DiskDriver_readBlock(d->sfs->disk, &ffb, file_block);
		ERROR_HELPER(ret, "Error in read block in file remove\n");
		
		if (memcmp(ffb.fcb.name, filename, strlen(ffb.fcb.name)) == 0) 	// file found
		remove_file_from_dir(d, read_entries, 0, file_block);

	}

	// scanning other blocks (not first one)
	while (read_entries < d->dcb->num_entries){

	}

}
*/