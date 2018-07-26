#include "simplefs.h"
#include <stdio.h>
#include "disk_driver.h"
#include <string.h>
#include <unistd.h>

#define TOTAL_BLOCKS 8192
// total space = 4194304


int main(int argc, char** argv) {
  /*printf("FirstBlock size %ld\n", sizeof(FirstFileBlock));
  printf("DataBlock size %ld\n", sizeof(FileBlock));
  printf("FirstDirectoryBlock size %ld\n", sizeof(FirstDirectoryBlock));
  printf("DirectoryBlock size %ld\n", sizeof(DirectoryBlock));
  */
  
  // INIT TEST ARGUMENTS AND COMMAND VALIDATION
  
  // note: even if we can choose which test run, most of them requires previous
  // ones for their structures. The argument only allows us to which test's outputs show
  int i,j,num_tests = 4,
      test_init = 0,
      test_get_free_block = 0,
      test_wrrd_free_block = 0,
      test_fs_init = 0;

  char* test_names[] = {"-test_init",
                       "-test_get_free_block",
                       "-test_wrrd_free_block",
                       "-test_fs_init"
                      }; 
  if (argc > 5){
    printf("usage: ./simplefs_test -<test1> -<test2> ... -<testn>\ntests available: \t-test_init\n\t-test_get_free_block\n\t-test_wrrd_free_block\n\t-test_fs_init\n");
    return -1;
  }
  for (i=1; i< argc; i++){
    int check = 0;
    for (j=0; j< num_tests; j++){
      if (memcmp(argv[i], test_names[j], strlen(argv[i])) == 0){
        printf("argv[i] = %s, test_names[j] = %s", argv[i], test_names[j]);
        if (j == 0) test_init = 1;
        if (j == 1) test_get_free_block = 1;
        if (j == 2) test_wrrd_free_block = 1;
        if (j == 3) test_fs_init = 1;
        check = 1;
        break;
      }
    }
    if (check == 0) {
      printf("usage: ./simplefs_test -<test1> -<test2> ... -<testn>\ntests available: \n\t-test_init\n\t-test_get_free_block\n\t-test_wrrd_free_block\n\t-test_fs_init\n");
      return -1;
    }
  }

  // if no argument passed, execute all tests
  if (argc == 1){
        test_init = 1;
        test_get_free_block = 1;
        test_wrrd_free_block = 1;
        test_fs_init = 1;
  }

  printf("init: %d, get: %d, wrrd: %d, fsinit: %d\n",test_init,test_get_free_block, test_wrrd_free_block, test_fs_init );
  // END COMMAND VALIDATION 

  int ret;
  
  // TEST INIT
  DiskDriver dd;
  DiskDriver_init(&dd, "mysystem", TOTAL_BLOCKS);
  DiskHeader* dh = dd.header;
  if (test_init){
    printf("---DiskHeader : TEST INIT \n\tnum_blocks = %d\n\tbitmap_blocks = %d\n\tbitmap_entries = %d\n\tfree_blocks = %d\n\tfirst_free_block = %d\n\n\n"
    , dh->num_blocks, dh->bitmap_blocks, dh->bitmap_entries, dh->free_blocks, dh->first_free_block);
  }  

  // TEST GET FREE BLOCK
  int free_block = DiskDriver_getFreeBlock(&dd, 1);
  if (test_get_free_block){
    printf("---DiskDriver : TEST getFreeBlock() \n\tfirst_free_block = %d \n\n\n",free_block);
  }

  //TEST WRITE FREE BLOCK - READBLOCK
  if (test_wrrd_free_block){ 
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
  }  
  /*
  ret = DiskDriver_writeBlock(&dd, buffer, 5);
  printf("Next free block: %d\n", DiskDriver_getFreeBlock(&dd, 0));
  ret = DiskDriver_writeBlock(&dd, buffer, 4);
  printf("Next free block: %d\n", DiskDriver_getFreeBlock(&dd, 0));
  */
  
  // TEST FS_INIT
  if (test_fs_init){
    printf("---SimpleFS : TEST FS_INIT \n");
    SimpleFS fs; 
    DirectoryHandle* dhandle = SimpleFS_init(&fs, &dd);
  }

  // TEST 


  
  // TEST CREATE FILE - OPEN FILE
  
  
  
  
}
