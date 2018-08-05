#include "simplefs.h"
#include <stdio.h>
#include "disk_driver.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define TOTAL_BLOCKS 8192
// total space = 4194304

#define ANSI_COLOR_PURPLE  "\e[1;35m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_GREEN   "\e[0;92m"
#define CHECK "✔"

int main(int argc, char** argv) {

  int ret;
  
  // TEST INIT
  DiskDriver dd;
  DiskDriver_init(&dd, "mysystem", TOTAL_BLOCKS);
  DiskHeader* dh = dd.header;
  printf(ANSI_COLOR_PURPLE "---DiskHeader : TEST INIT \n" ANSI_COLOR_RESET);
  printf("\tnum_blocks = %d\n\tbitmap_blocks = %d\n\tbitmap_entries = %d\n\tfree_blocks = %d\n\tfirst_free_block = %d\n\n\n", dh->num_blocks, dh->bitmap_blocks, dh->bitmap_entries, dh->free_blocks, dh->first_free_block);  

  // TEST GET FREE BLOCK
  int free_block = DiskDriver_getFreeBlock(&dd, 1);
  printf(ANSI_COLOR_PURPLE "---DiskDriver : TEST getFreeBlock()" ANSI_COLOR_RESET "\n\tfirst_free_block = %d. Expected 3. \n\n\n",free_block);

  //TEST WRITE FREE BLOCK - READBLOCK
  printf(ANSI_COLOR_PURPLE "---DiskDriver: TEST OF writeBlock() AND readBlock()\n\n\n" ANSI_COLOR_RESET);
  printf("\tCreating a buffer with some character stored...\n");
  char buffer[BLOCK_SIZE];
  memset(buffer,'-',BLOCK_SIZE);
  memcpy(buffer, "Prova di scrittura di un blocco, parte 1", 40);
  memcpy(buffer+100, "Prova di scritture di un blocco, parte 2", 40);
  printf("\tWriting in the first free block (block_num = %d) the content of the buffer...\n", free_block);
  ret = DiskDriver_writeBlock(&dd, buffer, free_block);
  printf("\tReading the content of the block just written...\n");
  char rblock[BLOCK_SIZE];
  ret |= DiskDriver_readBlock(&dd, rblock, free_block);
  if (ret != 0) printf("An error occurred in writeBlock or readBlock!\n\n");
  rblock[BLOCK_SIZE-1] = '\0'; 
  printf("\n\nBlocco letto:\n\n%s\n", rblock);

}