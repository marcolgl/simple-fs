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

int main(int argc, char** argv){

 int ret,i;

  printf(ANSI_COLOR_PURPLE "--SimpleFS : WEIRD CASES\n" ANSI_COLOR_RESET);
  printf("Here we test all those limit cases to check that the implementation works well even with big files/directories.\n");
  printf("In particular here we test the same functions in the simplefs_test but with directories and files contained in 2-3 blocks on disk.\n");

  // INIT THE DISK
  printf(ANSI_COLOR_CYAN "\nINIT THE DISK\n" ANSI_COLOR_RESET);
  DiskDriver dd;
  DiskDriver_init(&dd, "mysystem", TOTAL_BLOCKS); 
  SimpleFS fs; 
  DirectoryHandle* dhandle = SimpleFS_init(&fs, &dd);  
  printf("This is the dirhandle returned from init, it points to the root_directory\n");
  SimpleFS_printDirHandle(dhandle);

  // CREATE NEW FILE
  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : Create new file, read dir\n" ANSI_COLOR_RESET);
  printf("This tests create file in a big directory. To use a second block, we must add more than 87 file to it.\nIt also tests readDir reading the files on the big directory\n");
  printf(ANSI_COLOR_CYAN "\nADDING 112 FILES TO THE DIR\n" ANSI_COLOR_RESET);
  printf("Add file 'filo1' in '/' directory\n");
  printf("Add file 'filo2' in '/' directory\n");
  printf("Add file 'file0'-'file109' in '/' directory\n");
  FileHandle* qfh1 = SimpleFS_createFile(dhandle, "filo1");
  FileHandle* qfh2 = SimpleFS_createFile(dhandle, "filo2");
  int te = 0;
  FileHandle* qfhn;
  char filen[15];
  for (; te < 110; te++){
    snprintf(filen, 10, "file%d", te);
    qfhn = SimpleFS_createFile(dhandle, filen);
    SimpleFS_close(qfhn);
  }
  SimpleFS_close(qfh1);
  SimpleFS_close(qfh2);
  printf("\n-Reading the content of the '/' directory\n");
  char **namesto = malloc(sizeof(char*)*500);
  for (i=0; i< 112; i++) namesto[i] = malloc(sizeof(char)*128);
  ret = SimpleFS_readDir(namesto, dhandle);
  for (i=0; i < 112; i++){
    printf("\tfile %d: %s\n", i, namesto[i]);
    free(namesto[i]);
  }
  free(namesto);
  
  printf("\n-Current directory handle, first dir block and second dir block of '/' directory\n");
  SimpleFS_printDirHandle(dhandle);
  printf("\n");
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  printf("\n");
  SimpleFS_printDirBlock(dhandle, dhandle->current_block);
  
  // TEST FIND FILE IN DIR

  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST FIND FILE-DIR IN DIR \n" ANSI_COLOR_RESET);
  printf("Searching for file stored in the second block\n\n");
  printf("Searching file not in dir:\n");
  ret = SimpleFS_findFileInDir(dhandle, "notindir"); 
  printf("File trovato? %d. Expected: -1.", ret);
  if (ret == -1) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);
  printf("Searching file 'file108' (should be in dir):\n");
  ret = SimpleFS_findFileInDir(dhandle, "file108");
  printf("File trovato? %d. Expected: pos integer.", ret);
  if (ret > 0) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);

  // TEST CHANGEDIR-MKDIR

  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST MKDIR-CHANGEDIR \n" ANSI_COLOR_RESET);
  printf(ANSI_COLOR_CYAN "\nCREATING A NEW DIR IN '/'\n" ANSI_COLOR_RESET);
  printf("The newdir will be on the second dir block\n");
  printf("Creating a dir named: 'newdir'\n");
  ret = SimpleFS_mkDir(dhandle, "newdir");
  printf("\n-Current second block of '/'\n");
  SimpleFS_printDirBlock(dhandle, dhandle->current_block);
  printf("\n");
  printf("Moving to dir 'newdir'\n");
  ret = SimpleFS_changeDir(dhandle, "newdir");
  printf("-DirHandle, first dir block of 'newdir':\n");
  SimpleFS_printDirHandle(dhandle);
  printf("\n");
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  printf("\n");
  printf("Tree of content of 'newdir' (should be void):\n");
  printTree(dhandle);
  printf("Moving to dir '/'\n");
  SimpleFS_changeDir(dhandle, "..");
  printf("Tree of content of '/':\n");
  printTree(dhandle);
    
  // TEST FILE WRITE READ 2 


  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST FILE WRITE READ\n" ANSI_COLOR_RESET);
  printf("This tests the case in which we write and read a file that occupies 3 blocks.\n\n");
  char *buff2 = "Lo giorno se n’andava, e l’aere bruno\ntoglieva li animai che sono in terra\nda le fatiche loro; e io sol uno\nm’apparecchiava a sostener la guerra\nsi del cammino e si de la pietate,\nche ritrarra la mente che non erra.\nO muse, o alto ingegno, or m’aiutate;\no mente che scrivesti ciò ch’io vidi,\nqui si parra la tua nobilitate.\nIo cominciai: «Poeta che mi guidi,\nguarda la mia virtu s’ell’e possente,\nprima ch’a l’alto passo tu mi fidi.\nTu dici che di Silvio il parente,\ncorruttibile ancora, ad immortale\nsecolo andò, e fu sensibilmente.\nPero, se l’avversario d’ogne male\ncortese i fu, pensando l’alto effetto\nch’uscir dovea di lui e ’l chi e ’l quale,\nnon pare indegno ad omo d’intelletto;\nch’e’ fu de l’alma Roma e di suo impero\nne l’empireo ciel per padre eletto:\nla quale e ’l quale, a voler dir lo vero,\nfu stabilita per lo loco santo\nu’ siede il successor del maggior Piero.\nPer quest’andata onde li dai tu vanto,\nintese cose che furon cagione\ndi sua vittoria e del papale ammanto.\nAndovvi poi lo Vas d’elezione,\nper recarne conforto a quella fede\nch’e principio a la via di salvazione.\nMa io perche venirvi? o chi ’l concede?\nIo non Enea, io non Paulo sono:\nme degno a cio ne io ne altri ’l crede.";

  FileHandle* fh = SimpleFS_openFile(dhandle, "file1");
  SimpleFS_write(fh, buff2, strlen(buff2));
  printf("-Write some stuff inside file and print filehandle, first file block and last file block:\n\n");
  SimpleFS_printFileHandle(fh);
  printf("\n");
  SimpleFS_printFirstFileBlock(fh->fcb);
  printf("\n");
  SimpleFS_printFileBlock(fh->current_block);
  printf("\n\n");
  SimpleFS_close(fh);

  char read_buff3[2048];
  fh = SimpleFS_openFile(dhandle, "file1");
  SimpleFS_read(fh, read_buff3, 1100);
  printf("READ 1100 bytes on 3 blocks FROM FILE1:\n\n%s\n", read_buff3);
  printf("\n-Current filehandle:\n");
  SimpleFS_printFileHandle(fh);

  // ADDING SOME STUFF ON FILESYSTEM
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


  // TEST FILE REMOVE

  printf(ANSI_COLOR_PURPLE "\n\n---SimpleFS : TEST FILE REMOVE\n" ANSI_COLOR_RESET);
  printf("This module tests remove a file in a directory with 3 block(or more)\nNote that remove operation has to move an entry from last dir block to replace the entry removed.\n\n");
  printf("-Current tree from dir '/':\n\n");
  printTree(dhandle);
  printf("\nMoving to dir 'newdir'.");
  SimpleFS_changeDir(dhandle, "newdir");
  printf("-Current tree from dir 'newdir':\n\n");
  printTree(dhandle);
  
  printf(ANSI_COLOR_CYAN "\nADDING ENOUGH FILE TO REQEUST 4 BLOCK IN NEWDIR \n" ANSI_COLOR_RESET);
  printf("Creating 350 file on dir 'newdir':\n");
  char nam[10];
  FileHandle* fh2;
  for (i=0; i< 350; i++){
    snprintf(nam, 10, "file%d", i);
    fh2 = SimpleFS_createFile(dhandle, nam);
    SimpleFS_close(fh2);
  }
  printf("-Current content of newdir first block, second, third and last(4th) block\n\n");
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  printf("\n");
  DirectoryBlock dbtemp;
  DiskDriver_readBlock(dhandle->sfs->disk, &dbtemp, dhandle->dcb->header.next_block);
  SimpleFS_printDirBlock(dhandle, &dbtemp);
  printf("\n");
  DiskDriver_readBlock(dhandle->sfs->disk, &dbtemp, dbtemp.header.next_block);
  SimpleFS_printDirBlock(dhandle, &dbtemp);
  printf("\n");
  SimpleFS_printDirBlock(dhandle, dhandle->current_block);
  printf("\n");

  printf("-Current tree with root 'newdir': \n\n");
  printTree(dhandle);
  
  printf(ANSI_COLOR_CYAN "\nREMOVING A FILE FROM CURRENT DIR '\\'\n" ANSI_COLOR_RESET);
  printf("Removing 'file4' from '/' root dir\n");
  SimpleFS_remove(dhandle, "file4");

  printf("\n-After removing file 'temp' from dir:\n");
  printf("Check first and last block and see the swap between last block and block removed\n\n");
  SimpleFS_printFirstDirBlock(dhandle->dcb);
  printf("\n");
  SimpleFS_printDirBlock(dhandle, dhandle->current_block);
  printf("\n");

  printf("-Current tree with root 'newdir': \n\n");
  printTree(dhandle);
  
  printf("\nCurrent filesys tree with root '/':\n\n");
  SimpleFS_changeDir(dhandle, "..");
  printTree(dhandle);
  
  printf("\nNext_free_block: %d. Expected 130, cause i removed file4, whose block was 130.", DiskDriver_getFreeBlock(dhandle->sfs->disk, 0));
  if (DiskDriver_getFreeBlock(&dd, 0) == 130) printf(ANSI_COLOR_GREEN " ✓\n" ANSI_COLOR_RESET);

}