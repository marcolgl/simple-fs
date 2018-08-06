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
#define ANSI_COLOR_CYAN    "\e[1;36m"   



int main(int argc, char** argv) {
  
  // INIT TEST ARGUMENTS AND COMMAND VALIDATION
  
  // note: even if we can choose which test run, most of them requires previous
  // ones for their structures. The argument only allows us to which test's outputs show
  int i,j,num_tests = 11,
      test_init = 0,
      test_format = 0,
      test_create_file = 0,
      test_read_dir = 0,
      test_mkdir = 0,
      test_find = 0,
      test_cd = 0,
      test_print_tree = 0,
      test_open_close = 0,
      test_write_read_seek = 0,
      test_remove = 0;

  char* test_names[] = {
                       "-test_init",
                       "-test_format",
                       "-test_create_file",
                       "-test_read_dir",
                       "-test_mkdir",
                       "-test_find",
                       "-test_cd",
                       "-test_print_tree",
                       "-test_open_close",
                       "-test_write_read_seek",
                       "-test_remove"
                      }; 
  if (argc > 10){
    printf("usage: ./simplefs_test -<test1> -<test2> ... -<testn>\ntests available: \n\t-test_init\n\t-test_format\n\t-test_create_file\n\t-test_read_dir\n\t-test_mkdir\n\t-test_find\n\t-test_cd\n\t-test_print_tree\n\t-test_open_close\n\t-test_write_read_seek\n\t-test_remove\n");
    return -1;
  }
  for (i=1; i< argc; i++){
    int check = 0;
    for (j=0; j< num_tests; j++){
      if (memcmp(argv[i], test_names[j], max(strlen(argv[i]), strlen(test_names[j]))) == 0){
        if (j == 0) test_init = 1;
        if (j == 1) test_format = 1;
        if (j == 2) test_create_file = 1;
        if (j == 3) test_read_dir = 1;
        if (j == 4) test_mkdir = 1;
        if (j == 5) test_find = 1;
        if (j == 6) test_cd = 1;
        if (j == 7) test_print_tree = 1;
        if (j == 8) test_open_close = 1;
        if (j == 9) test_write_read_seek = 1;
        if (j == 10) test_remove = 1;
        check = 1;
        break;
      }
    }
    if (check == 0) {
      printf("usage: ./simplefs_test -<test1> -<test2> ... -<testn>\ntests available: \n\t-test_init\n\t-test_format\n\t-test_create_file\n\t-test_read_dir\n\t-test_mkdir\n\t-test_find\n\t-test_cd\n\t-test_print_tree\n\t-test_open_close\n\t-test_write_read_seek\n\t-test_remove\n");
      return -1;
    }
  }

  // if no argument passed, execute all tests
  if (argc == 1){
        test_init = 1;
        test_format = 1;
        test_create_file = 1;
        test_read_dir = 1;
        test_mkdir = 1;
        test_find = 1;
        test_cd = 1;
        test_print_tree = 1;
        test_open_close = 1;
        test_write_read_seek = 1;
        test_remove = 1;
  }
  // END COMMAND VALIDATION 

  int ret;

  // INIT THE DISK
  DiskDriver dd;
  DiskDriver_init(&dd, "mysystem", TOTAL_BLOCKS); 
  
  // TEST FS_INIT
  if (test_init){
    printf(ANSI_COLOR_PURPLE "---SimpleFS : TEST FS_INIT \n" ANSI_COLOR_RESET);
  }
    SimpleFS fs; 
    DirectoryHandle* dhandle = SimpleFS_init(&fs, &dd);
  if (test_init){  
    printf("This is the dirhandle returned from init, it points to the root_directory\n");
    SimpleFS_printDirHandle(dhandle);
    printf("root_dir block num: %d. Expected 3.", dhandle->block_num);
    if (dhandle->block_num == 3) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  }

  // TEST FS_FORMAT
  if (test_format){
    printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST FS_FORMAT \n" ANSI_COLOR_RESET);

    // Dirty one block on disk
    char buffer[BLOCK_SIZE];
    ret = DiskDriver_writeBlock(&dd, buffer, DiskDriver_getFreeBlock(&dd, 0));
    ret = DiskDriver_getFreeBlock(&dd, 0);
    printf("Dirty one block on disk, writing on the next free block.\nNext free block should now be 5th block if write succeeded.\nNext free block: %d. Expected 5", ret);
    if (ret == 5) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
    printf("Formatting Filesystem, the blocks in use for data should be released. Note that 3 blocks are needed for metadata and 1 for root directory. So result dir handle should be the same as after init.\n\n");
    SimpleFS_format(&fs);
    SimpleFS_printDirHandle(dhandle);
    ret = DiskDriver_getFreeBlock(&dd, 0);
    printf("Next free block should now be the 4th if the format succeeded.\nNext free block: %d. Expected 4.", ret);
    if (ret == 4) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  }

    // TEST CREATE NEW FILE 000
/*
  printf("\n\n---SimpleFS : Create new file test case 000 \n");
  printf("Add a files in '/' directory\n");
  FileHandle* qfh1 = SimpleFS_createFile(dhandle, "filo1");
  FileHandle* qfh2 = SimpleFS_createFile(dhandle, "filo2");
  int te = 0;
  FileHandle* qfhn;
  char filen[15];
  for (; te < 110; te++){
    snprintf(filen, 10, "file%d", te);
    qfhn = SimpleFS_createFile(dhandle, filen);
  }
  SimpleFS_close(qfh1);
  SimpleFS_close(qfh2);

  char **namesto = malloc(sizeof(char*)*500);
  for (i=0; i< 112; i++) namesto[i] = malloc(sizeof(char)*128);
  ret = SimpleFS_readDir(namesto, dhandle);
  for (i=0; i < 112; i++){
    printf("\tfile %d: %s\n", i, namesto[i]);
  }

  
  FirstFileBlock fabbrica1;
  printf("Dati nel blocco 2 della dir: %d\n",dhandle->current_block->file_blocks[0]);
  ret = DiskDriver_readBlock(dhandle->sfs->disk, &fabbrica1, 89);
	printf("name of file: %s\n", fabbrica1.fcb.name);

  DirectoryBlock como1;
  ret = DiskDriver_readBlock(dhandle->sfs->disk, &como1, 88);
	printf("first entry: %d\n", como1.file_blocks[3]);
  

  printf("Errore=\n");
  //return 1;
  printTree(dhandle);
  return 1;
    //
*/


  // TEST CREATE NEW FILE 
if(test_create_file){
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST CREATE NEW FILE IN DIR \n" ANSI_COLOR_RESET);
}
  printf(ANSI_COLOR_CYAN "\nADDING SOME FILE TO THE FILESYSTEM\n" ANSI_COLOR_RESET);
  printf("Add a file 'temp' in '/' directory\n");
  FileHandle* fhandle1 = SimpleFS_createFile(dhandle, "temp");
  printf("Add a file 'proj' in '/' directory\n");
  FileHandle* fhandle2 = SimpleFS_createFile(dhandle, "proj");
  printf("Add a file 'proj' in '/' again. It shouldn't be added cause already exists\n");
  SimpleFS_createFile(dhandle, "proj");
if (test_create_file){
  printf("\nThese are the file handler returned from create operation:\n\n");
  SimpleFS_printFileHandle(fhandle1);
  SimpleFS_printFileHandle(fhandle2);
}

  // TEST READ DIR 
if (test_read_dir){
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST READ DIR\n" ANSI_COLOR_RESET);
  printf("Reading '%s' directory: \n", dhandle->dcb->fcb.name);
  char **names = malloc(sizeof(char*)*128);
  for (i=0; i< 128; i++) names[i] = malloc(sizeof(char)*128);
  ret = SimpleFS_readDir(names, dhandle);
  for (i=0; i < ret; i++){
    printf("\tfile %d: %s\n", i, names[i]);
  }
  printf("Expected '/' to contain 'temp' and 'proj'.");
  if (memcmp(names[0],"temp", max(strlen("temp"),strlen(names[0]))) == 0 && memcmp(names[1],"proj", max(strlen("proj"),strlen(names[1]))) == 0)
    printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  // free memory
  for (i=0; i< 128; i++){
    free(names[i]);
  }
  free(names);
}

  // TEST MKDIR
if (test_mkdir){
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST MKDIR\n" ANSI_COLOR_RESET);
}
  printf(ANSI_COLOR_CYAN "\nCREATING A NEWDIR\n" ANSI_COLOR_RESET);
  printf("Creating a dir named: 'newdir'\n");
  ret = SimpleFS_mkDir(dhandle, "newdir");
if (test_mkdir){  
  printf("Directory created? %d. Expected 0 (success)", ret);
  if (ret == 0) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  char ** dirnames = malloc(sizeof(char*)*128);
  for (i=0; i< 128; i++) dirnames[i] = malloc(sizeof(char)*128);
  ret = SimpleFS_readDir(dirnames, dhandle);
  for (i=0; i < ret; i++){
    printf("\tfile %d: %s\n", i, dirnames[i]);
  }
  printf("Expected '/' to contain 'temp', 'proj' and 'newdir'.\n");
}

  // TEST FIND FILE-DIR IN DIR 
if (test_find){ 
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST FIND FILE-DIR IN DIR \n" ANSI_COLOR_RESET);
  printf("Searching file not in dir:\n");
  ret = SimpleFS_findFileInDir(dhandle, "notindir"); 
  printf("File trovato? %d. Expected: -1.", ret);
  if (ret == -1) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  printf("Searching file 'temp' (should be in dir):\n");
  ret = SimpleFS_findFileInDir(dhandle, "temp");
  printf("File trovato? %d. Expected: pos integer.", ret);
  if (ret > 0) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  printf("Searching file 'proj' (should be in dir):\n");
  ret = SimpleFS_findFileInDir(dhandle, "proj");
  printf("File trovato? %d. Expected: pos integer.", ret);
  if (ret > 0) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  printf("Searching directory 'newdir' (should be in dir):\n");
  ret = SimpleFS_findDirInDir(dhandle, "newdir");
  printf("Directory trovata? %d. Expected: pos integer.", ret);
  if (ret > 0) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
}
  
  // TEST CHANGE DIR
if(test_cd){
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST CHANGE DIR\n" ANSI_COLOR_RESET);
  printf("-Current dirhandle e first dir block: \n");
  SimpleFS_printDirHandle(dhandle);
  printf("Block_num expected: 3");
  if (dhandle->block_num == 3) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  
  printf("\n\n");

  printf("-Change directory, moving from '/' to 'newdir':\n");
  SimpleFS_changeDir(dhandle, "newdir");
  SimpleFS_printDirHandle(dhandle);
  printf("Block_num expected: 6");
  if (dhandle->block_num == 6) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  
  printf("\n\n");

  printf("-Moving again to '\\'\n");
  SimpleFS_changeDir(dhandle, "..");
  SimpleFS_printDirHandle(dhandle);
  SimpleFS_printFirstDirBlock(dhandle->dcb);
}

  // TEST PRINT TREE

if (test_print_tree){  
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST PRINT TREE\n" ANSI_COLOR_RESET);
}
  printf(ANSI_COLOR_CYAN "\nADDING SOME FILE TO THE FILESYSTEM\n" ANSI_COLOR_RESET);
  printf("Add a file 'stuff' in '/' directory\n");
  printf("Add a file 'void.txt' in '/' directory\n");
  SimpleFS_createFile(dhandle, "stuff");
  SimpleFS_createFile(dhandle, "void.txt");
  printf("\n");
  
if (test_print_tree){
  printf("-Current dirhandle e first dir block: \n");
  SimpleFS_printDirHandle(dhandle);
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  printf("Entries_dir: %d. Expected 5.", dhandle->dcb->num_entries);
  if (dhandle->dcb->num_entries == 5) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  printf("\n-Printing tree starting from root '/' dir:\n");  
  printTree(dhandle);
}

  printf(ANSI_COLOR_CYAN "\nADDING SOME OTHER FILE AND DIRECTORY TO THE FILESYSTEM\n" ANSI_COLOR_RESET);
  printf("Add a file 'index.txt' in '/' directory\n");
  printf("Add a directory 'newdir2' in '/' directory\n");
  printf("Move to directory 'newdir2'\n");
  printf("Add a file 'proj' in 'newdir2' directory\n");
  printf("Add a directory 'newdir3' in 'newdir2' directory\n");
  printf("Move to directory 'newdir3'\n");
  printf("Add a file 'music-sheets.txt' in 'newdir3' directory\n");
  printf("Move to root directory '/'\n");
  SimpleFS_createFile(dhandle, "index.txt");
  SimpleFS_mkDir(dhandle, "newdir2");
  SimpleFS_changeDir(dhandle, "newdir2");
  SimpleFS_createFile(dhandle,"proj");
  SimpleFS_mkDir(dhandle, "newdir3");
  SimpleFS_changeDir(dhandle, "newdir3");
  SimpleFS_createFile(dhandle, "music-sheets.txt");
  SimpleFS_changeDir(dhandle, "..");
  SimpleFS_changeDir(dhandle, "..");

if (test_print_tree){
  printf("\n-Printing tree starting from root '/' dir:\n");  
  printTree(dhandle);
  printf("\n-Current dirhandle e first dir block:\nNote the blocks corresponding to the file and dirs added to root.\n");
  SimpleFS_printDirHandle(dhandle);
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  printf("Entries_dir: %d. Expected 7.", dhandle->dcb->num_entries);
  if (dhandle->dcb->num_entries == 7) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
}

  // TEST FILE OPEN - CLOSE

if (test_open_close){
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST FILE OPEN - CLOSE\n" ANSI_COLOR_RESET);
  printf("-Open file 'temp', it should return a FileHandle:\n");
  FileHandle* fh = SimpleFS_openFile(dhandle, "temp");
  SimpleFS_printFileHandle(fh);
  printf("Expected block_num = 4");
  if (fh->block_num == 4) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  SimpleFS_printFirstFileBlock(fh->fcb);
  printf("File close (it only frees the memory).\n");
  SimpleFS_close(fh);
}

if (test_write_read_seek){
  // TEST FILE WRITE
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST FILE WRITE-READ\n" ANSI_COLOR_RESET);
  char *buff = "Il cammino dell'uomo timorato e minacciato da ogni parte dalle iniquita degli esseri egoisti e dalla tirannia degli uomini malvagi, benedetto sia colui che conduce i deboli attraverso la valle delle tenebre, poiche egli e il pastore di suo fratello ed il ricercatore dei figli smarriti. E la mia vendetta calera su di loro, con grandissima malavagita e furiosissimo sdegno, du si coloro che si provanno ad ammorbare e a distruggere i miei fratelli, e tu saprai che il mio nome e quello del signore, quando faro calare la mia vendetta sopra di te.";
  FileHandle* fh = SimpleFS_openFile(dhandle, "temp");
  printf("-Writing 546 bytes on 'temp' file, previously void.\nIt should take another block to fulfil the request.\n");
  SimpleFS_write(fh, buff, strlen(buff));
  printf("\n-Filehandle, first file block e current file block:\n");
  SimpleFS_printFileHandle(fh);
  SimpleFS_printFirstFileBlock(fh->fcb);
  SimpleFS_printFileBlock(fh->current_block);
  SimpleFS_close(fh);
  // TEST FILE READ
  char read_buff[2048];
  memset(read_buff, 0, sizeof(read_buff));
  fh = SimpleFS_openFile(dhandle, "temp");
  SimpleFS_read(fh, read_buff, 440);
  printf("\n-Read first 440 bytes of 'temp' and store them in a buffer:\n%s\n\n", read_buff);
  SimpleFS_printFileHandle(fh);
  SimpleFS_read(fh, read_buff+440, 50);
  printf("\nRead next 50 bytes on file and append them to the buffer:\n%s\n\n", read_buff);
  SimpleFS_printFileHandle(fh);
  SimpleFS_close(fh);


  // TEST FILE WRITE READ 2 
/*
    char read_buff2[2048];
    printf("\n\n---SimpleFS : TEST FILE WRITE READ 2\n");
  char *buff2 = "Lo giorno se n’andava, e l’aere bruno\ntoglieva li animai che sono in terra\nda le fatiche loro; e io sol uno\nm’apparecchiava a sostener la guerra\nsi del cammino e si de la pietate,\nche ritrarra la mente che non erra.\nO muse, o alto ingegno, or m’aiutate;\no mente che scrivesti ciò ch’io vidi,\nqui si parra la tua nobilitate.\nIo cominciai: «Poeta che mi guidi,\nguarda la mia virtu s’ell’e possente,\nprima ch’a l’alto passo tu mi fidi.\nTu dici che di Silvio il parente,\ncorruttibile ancora, ad immortale\nsecolo andò, e fu sensibilmente.\nPero, se l’avversario d’ogne male\ncortese i fu, pensando l’alto effetto\nch’uscir dovea di lui e ’l chi e ’l quale,\nnon pare indegno ad omo d’intelletto;\nch’e’ fu de l’alma Roma e di suo impero\nne l’empireo ciel per padre eletto:\nla quale e ’l quale, a voler dir lo vero,\nfu stabilita per lo loco santo\nu’ siede il successor del maggior Piero.\nPer quest’andata onde li dai tu vanto,\nintese cose che furon cagione\ndi sua vittoria e del papale ammanto.\nAndovvi poi lo Vas d’elezione,\nper recarne conforto a quella fede\nch’e principio a la via di salvazione.\nMa io perche venirvi? o chi ’l concede?\nIo non Enea, io non Paulo sono:\nme degno a cio ne io ne altri ’l crede.";

  //memset(buff, 'V', strlen(buff));
  fh = SimpleFS_openFile(dhandle, "proj");
  SimpleFS_write(fh, buff2, strlen(buff2));
  SimpleFS_printFileHandle(fh);
  SimpleFS_printFirstFileBlock(fh->fcb);
  SimpleFS_printFileBlock(fh->current_block);
  //SimpleFS_close(fh);

  char read_buff3[2048];
  //memset(read_buff3, 0, sizeof(read_buff3));
  fh = SimpleFS_openFile(dhandle, "proj");
  SimpleFS_read(fh, read_buff3, 1100);
  printf("READ FROM FILE TEMP:\n%s\n", read_buff3);
  SimpleFS_printFileHandle(fh);
  //SimpleFS_read(fh, read_buff+40, 50);
  //printf("READ FROM FILE TEMP:\n%s\n", read_buff);
  //SimpleFS_printFileHandle(fh);
  return 1;
*/


  // TEST FILE SEEK
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST FILE SEEK\n" ANSI_COLOR_RESET);
  char buffer[2048];
  fh = SimpleFS_openFile(dhandle, "temp");
  printf("-Seek to position 400 in file 'temp'.\n");
  SimpleFS_seek(fh, 400);
  SimpleFS_read(fh, buffer, 15);
  printf("Read 15 bytes from current position:\n%s-\n\n", buffer);
  printf("FileHandle after seek and read:");
  SimpleFS_printFileHandle(fh);
  printf("Expected pos_in_file: 75");
  if (fh->pos_in_file == 75) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  SimpleFS_close(fh);
}

  // TEST FILE REMOVE
if (test_remove){
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST FILE REMOVE\n" ANSI_COLOR_RESET);
  printf("-Before removing file 'temp' from dir:\n");
  /*char nam[10];
  FileHandle* fh2;
  // i = 101
  for (i=0; i< 350; i++){
    snprintf(nam, 10, "file%d", i);
    fh2 = SimpleFS_createFile(dhandle, nam);
    SimpleFS_close(fh2);
  }
*/
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  printf("\n");
  //SimpleFS_printDirBlock(dhandle, dhandle->current_block);
  printTree(dhandle);
  }
  
  printf(ANSI_COLOR_CYAN "\nREMOVING A FILE FROM CURRENT DIR '\\'\n" ANSI_COLOR_RESET);
  printf("Removing 'temp' from '/' root dir\n");
  SimpleFS_remove(dhandle, "temp");

if (test_remove){
  printf("\n-After removing file 'temp' from dir:\n");
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  printf("\n");
  //SimpleFS_printDirBlock(dhandle, dhandle->current_block);
  printTree(dhandle);
  //SimpleFS_changeDir(dhandle, "newdir");
  //printTree(dhandle);
  printf("\n");
  printf("Next_free_block: %d. Expected 4, cause i removed temp, whose block was 4.", DiskDriver_getFreeBlock(dhandle->sfs->disk, 0));
  if (DiskDriver_getFreeBlock(&dd, 0) == 4) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  //printf("Next_free_block: %d\n", DiskDriver_getFreeBlock(dhandle->sfs->disk, 5));
  //printf("Next_free_block: %d\n", DiskDriver_getFreeBlock(dhandle->sfs->disk, 8)); 
  }

}
