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
// AGG:
void SimpleFS_printDirBlock(DirectoryHandle* d, DirectoryBlock* db);




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

int max(int a, int b);

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


// TODO LIST
//1) remove case: remove dir
//2) fix: case directory longer than 2 blocks
	    // forse errore nella bitmap, alloca blocchi a random, passa da 129 a 136 come libero
		// errore nella bitmap, non setta i blocchi dopo 129
		// solved: facevo parse da int a char di (129) invece di farlo di (129%8)
//3) fix: fails to allocate the 3rd directory block when  needed, so i have an error in findfileindir when searching for a filename that should be in 3rd block