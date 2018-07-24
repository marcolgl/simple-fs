#include "simplefs.h"
#include <stdio.h>
#include "disk_driver.h"
#include <string.h>
#include <unistd.h>

#define TOTAL_BLOCKS 8192
// total space = 4194304

int main(int agc, char** argv) {
  /*printf("FirstBlock size %ld\n", sizeof(FirstFileBlock));
  printf("DataBlock size %ld\n", sizeof(FileBlock));
  printf("FirstDirectoryBlock size %ld\n", sizeof(FirstDirectoryBlock));
  printf("DirectoryBlock size %ld\n", sizeof(DirectoryBlock));
  */
  
  int ret;
  
  // TEST INIT
  DiskDriver dd;
  DiskDriver_init(&dd, "mysystem", TOTAL_BLOCKS);
  DiskHeader* dh = dd.header;
  printf("---DiskHeader : TEST INIT \n\tnum_blocks = %d\n\tbitmap_blocks = %d\n\tbitmap_entries = %d\n\tfree_blocks = %d\n\tfirst_free_block = %d\n\n\n"
  , dh->num_blocks, dh->bitmap_blocks, dh->bitmap_entries, dh->free_blocks, dh->first_free_block);
  

  // TEST GET FREE BLOCK
  int free_block = DiskDriver_getFreeBlock(&dd, 1);
  printf("---DiskDriver : TEST getFreeBlock() \n\tfirst_free_block = %d \n\n\n",free_block);
  
  //TEST WRITE FREE BLOCK - READBLOCK
  printf("---DiskDriver: TEST OF writeBlock() AND readBlock()\n\n\n");
  printf("Creating a buffer with some character stored...\n\n");
  sleep(2);
  char buffer[BLOCK_SIZE];
  memset(buffer,'-',BLOCK_SIZE);
  memcpy(buffer, "Prova di scrittura di un blocco, parte 1", 40);
  memcpy(buffer+100, "Prova di scritture di un blocco, parte 2", 40);
  printf("Writing in the first free block (block_num = %d) the content of the buffer...\n\n", free_block);
  sleep(2);
  ret = DiskDriver_writeBlock(&dd, buffer, free_block);
  printf("Reading the content of the block just written...\n\n");
  sleep(2);
  char rblock[BLOCK_SIZE];
  ret |= DiskDriver_readBlock(&dd, rblock, free_block);
  if (ret != 0) printf("An error occurred in writeBlock or readBlock!\n\n");
  rblock[BLOCK_SIZE-1] = '\0'; 
  printf("Blocco letto:\n%s\n\n\n", rblock);
  printf("DiskDriver: End test di writeBlock() e readBlock().\n\n\n\n");
  
  /*
  ret = DiskDriver_writeBlock(&dd, buffer, 5);
  printf("Next free block: %d\n", DiskDriver_getFreeBlock(&dd, 0));
  ret = DiskDriver_writeBlock(&dd, buffer, 4);
  printf("Next free block: %d\n", DiskDriver_getFreeBlock(&dd, 0));
  */
  
  //
  
  
  // TEST CREATE FILE - OPEN FILE
  
  
  
  
}
