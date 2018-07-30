#include "simplefs.h"
#include <stdio.h>
#include "disk_driver.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

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
  int i,j,num_tests = 6,
      test_init = 0,
      test_get_free_block = 0,
      test_wrrd_free_block = 0,
      test_fs_init = 0,
      test_fs_format = 0,
      test_print_tree = 0;

  char* test_names[] = {"-test_init",
                       "-test_get_free_block",
                       "-test_wrrd_free_block",
                       "-test_fs_init",
                       "-test_fs_format",
                       "-test_print_tree"
                      }; 
  if (argc > 6){
    printf("usage: ./simplefs_test -<test1> -<test2> ... -<testn>\ntests available: \t-test_init\n\t-test_get_free_block\n\t-test_wrrd_free_block\n\t-test_fs_init\n\t-test_print_tree\n");
    return -1;
  }
  for (i=1; i< argc; i++){
    int check = 0;
    for (j=0; j< num_tests; j++){
      if (memcmp(argv[i], test_names[j], strlen(test_names[j])) == 0){
        if (j == 0) test_init = 1;
        if (j == 1) test_get_free_block = 1;
        if (j == 2) test_wrrd_free_block = 1;
        if (j == 3) test_fs_init = 1;
        if (j == 4) test_fs_format = 1;
        if (j == 5) test_print_tree = 1;
        check = 1;
        break;
      }
    }
    if (check == 0) {
      printf("usage: ./simplefs_test -<test1> -<test2> ... -<testn>\ntests available: \n\t-test_init\n\t-test_get_free_block\n\t-test_wrrd_free_block\n\t-test_fs_init\n\t-test_fs_format\n");
      return -1;
    }
  }

  // if no argument passed, execute all tests
  if (argc == 1){
        test_init = 1;
        test_get_free_block = 1;
        test_wrrd_free_block = 1;
        test_fs_init = 1;
        test_fs_format = 1;
        test_print_tree = 1;
  }
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
    printf("---DiskDriver : TEST getFreeBlock() \n\tfirst_free_block = %d. Expected 3 \n\n\n",free_block);
  }

  //TEST WRITE FREE BLOCK - READBLOCK
  if (test_wrrd_free_block){ 
    printf("---DiskDriver: TEST OF writeBlock() AND readBlock()\n\n\n");
    printf("Creating a buffer with some character stored...\n\n");
    //sleep(2);
    char buffer[BLOCK_SIZE];
    memset(buffer,'-',BLOCK_SIZE);
    memcpy(buffer, "Prova di scrittura di un blocco, parte 1", 40);
    memcpy(buffer+100, "Prova di scritture di un blocco, parte 2", 40);
    printf("Writing in the first free block (block_num = %d) the content of the buffer...\n\n", free_block);
    //sleep(2);
    ret = DiskDriver_writeBlock(&dd, buffer, free_block);
    printf("Reading the content of the block just written...\n\n");
    //sleep(2);
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
  }
    SimpleFS fs; 
    DirectoryHandle* dhandle = SimpleFS_init(&fs, &dd);
  if (test_fs_init){  
    SimpleFS_printDirHandle(dhandle);
  }

  // TEST FS_FORMAT
  if (test_fs_format){
    printf("\n\n---SimpleFS : TEST FS_FORMAT \n");
    ret = DiskDriver_getFreeBlock(&dd,0);
    printf("Next free block: %d.\n", ret);
    printf("Formatting Filesystem, the blocks in use for data should be released. Note that 3 blocks are needed for metadata and 1 for root directory\n\n");
    SimpleFS_format(&fs);
    ret = DiskDriver_getFreeBlock(&dd,0);
    printf("Next free block: %d. Expected: 4\n", ret);
  }

  
  // TEST CREATE NEW FILE 
  printf("\n\n---SimpleFS : TEST CREATE NEW FILE IN DIR \n");
  printf("Add a file 'temp' in '/' directory\n");
  FileHandle* fhandle1 = SimpleFS_createFile(dhandle, "temp");
  printf("Add a file 'proj' in '/' directory\n");
  FileHandle* fhandle2 = SimpleFS_createFile(dhandle, "proj");
  printf("Add a file 'proj' in '/' again. It shouldn't be added cause already exists\n");
  SimpleFS_createFile(dhandle, "proj");
  SimpleFS_printFileHandle(fhandle1);
  SimpleFS_printFileHandle(fhandle2);

  // TEST READ DIR 
  printf("\n\n---SimpleFS : TEST READ DIR\n");
  printf("Reading '%s' directory: \n", dhandle->dcb->fcb.name);
  char **names = malloc(sizeof(char*)*128);
  for (i=0; i< 128; i++) names[i] = malloc(sizeof(char)*128);
  ret = SimpleFS_readDir(names, dhandle);
  for (i=0; i < ret; i++){
    printf("\tfile %d: %s\n", i, names[i]);
  }
  printf("Expected '/' to contain 'temp' and 'proj'.\n");
  printf("End of TEST READ DIR\n\n");


  // TEST MKDIR
  printf("\n\n---SimpleFS : TEST MKDIR\n");
  printf("Creating a dir named: 'newdir'\n");
  ret = SimpleFS_mkDir(dhandle, "newdir");
  printf("Directory created? %d. Expected 0 (success)\n", ret);
  // free the structure
  for (i=0; i < ret; i++){
    free(names[i]);
    names[i] = malloc(sizeof(char)*128);
  }
  ret = SimpleFS_readDir(names, dhandle);
  for (i=0; i < ret; i++){
    printf("\tfile-dir %d: %s\n", i, names[i]);
  }
  printf("Expected '/' to contain 'temp', 'proj' and 'newdir'.\n");
  printf("End of test MKDIR\n\n");



  // TEST FIND FILE-DIR IN DIR 
    // note: will have to test more after adding some file in dir
  printf("\n\n---SimpleFS : TEST FIND FILE-DIR IN DIR \n");
  printf("Searching file not in dir:\n");
  ret = SimpleFS_findFileInDir(dhandle, "notindir"); 
  printf("File trovato? %d. Expected: -1\n", ret);
  ret = SimpleFS_findFileInDir(dhandle, "temp");
  printf("File trovato? %d. Expected: pos integer\n", ret);
  ret = SimpleFS_findFileInDir(dhandle, "proj");
  printf("File trovato? %d. Expected: pos integer\n", ret);
  ret = SimpleFS_findDirInDir(dhandle, "newdir");
  printf("File trovato? %d. Expected: pos integer\n", ret);

  
  // TEST CHANGE DIR
  printf("\n\n---SimpleFS : TEST CHANGE DIR\n");
  printf("Change directory, moving from '/' to 'newdir':\n");
  SimpleFS_printDirHandle(dhandle);
  printf("block_num expected: 4\n");
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  
  FirstDirectoryBlock fdbtemp;
  DiskDriver_readBlock(&dd, &fdbtemp, 3);
  SimpleFS_printFirstDirBlock(&fdbtemp);

  SimpleFS_changeDir(dhandle, "newdir");
  SimpleFS_printDirHandle(dhandle);
  printf("block_num expected: \n");
  SimpleFS_printFirstDirBlock(dhandle->dcb);

  // TEST PRINT TREE

if (test_print_tree){  
  printf("\n\n---SimpleFS : TEST PRINT TREE\n");
  SimpleFS_createFile(dhandle, "stuff");
  SimpleFS_createFile(dhandle, "void.txt");
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  SimpleFS_changeDir(dhandle, "..");
  SimpleFS_printDirHandle(dhandle);
  printf("entries_dir: %d\n", dhandle->dcb->num_entries);
  printf("Printing tree with root '/' dir:\n");  
  printTree(dhandle);
  for (i=0; i < ret; i++){
    free(names[i]);
    names[i] = malloc(sizeof(char)*128);
  }
  ret = SimpleFS_readDir(names, dhandle);
  for (i=0; i < 3; i++){
    printf("\tfile-dir %d: %s\n", i, names[i]);
  }

  ret = SimpleFS_findFileInDir(dhandle, "proj");
  printf("File trovato? %d. Expected: pos integer\n", ret);
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  
  printTree(dhandle);
  SimpleFS_printFirstDirBlock(dhandle->dcb);;

  SimpleFS_createFile(dhandle, "index.txt");
  SimpleFS_mkDir(dhandle, "newdir2");
  SimpleFS_changeDir(dhandle, "newdir2");
  SimpleFS_createFile(dhandle,"proj");
  SimpleFS_mkDir(dhandle, "newdir3");
  SimpleFS_changeDir(dhandle, "newdir3");
  SimpleFS_createFile(dhandle, "music-sheets.txt");
  SimpleFS_changeDir(dhandle, "..");
  SimpleFS_changeDir(dhandle, "..");
  printTree(dhandle);
  SimpleFS_printFirstDirBlock(dhandle->dcb);
} 

  // TEST FILE OPEN - CLOSE

  printf("\n\n---SimpleFS : TEST FILE OPEN - CLOSE\n");
  SimpleFS_changeDir(dhandle, "..");
  printTree(dhandle);
  FileHandle* fh = SimpleFS_openFile(dhandle, "temp");
  SimpleFS_printFileHandle(fh);
  printf("Expected block_num = 4\n");
  SimpleFS_printFirstFileBlock(fh->fcb);
  SimpleFS_close(fh);

  // TEST FILE WRITE

  printf("\n\n---SimpleFS : TEST FILE WRITE\n");
  char *buff = "Il cammino dell'uomo timorato e minacciato da ogni parte dalle iniquita degli esseri egoisti e dalla tirannia degli uomini malvagi, benedetto sia colui che conduce i deboli attraverso la valle delle tenebre, poiche egli e il pastore di suo fratello ed il ricercatore dei figli smarriti. E la mia vendetta calera su di loro, con grandissima malavagita e furiosissimo sdegno, du si coloro che si provanno ad ammorbare e a distruggere i miei fratelli, e tu saprai che il mio nome e quello del signore, quando faro calare la mia vendetta sopra di te.";
  //memset(buff, 'V', strlen(buff));
  fh = SimpleFS_openFile(dhandle, "temp");
  SimpleFS_write(fh, buff, strlen(buff));
  SimpleFS_printFileHandle(fh);
  SimpleFS_printFirstFileBlock(fh->fcb);
  SimpleFS_printFileBlock(fh->current_block);
  //SimpleFS_close(fh);


  // TEST FILE READ
/*
  printf("\n\n---SimpleFS : TEST FILE READ\n");
  char read_buff[2048];
  memset(read_buff, 0, sizeof(read_buff));
  char read_buff2[2048];
  fh = SimpleFS_openFile(dhandle, "temp");
  SimpleFS_read(fh, read_buff, 440);
  printf("READ FROM FILE TEMP:\n%s\n", read_buff);
  SimpleFS_printFileHandle(fh);
  //SimpleFS_read(fh, read_buff+40, 50);
  //printf("READ FROM FILE TEMP:\n%s\n", read_buff);
  //SimpleFS_printFileHandle(fh);
*/

  // TEST FILE SEEK
  printf("\n\n---SimpleFS : TEST FILE SEEK\n");
  char buffer[2048];
  SimpleFS_seek(fh, 400);
  SimpleFS_read(fh, buffer, 15);
  printf("READ STUFF: \n%s-\n", buffer);
  SimpleFS_close(fh);

}
