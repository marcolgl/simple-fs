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

 int ret;

  printf(ANSI_COLOR_PURPLE "--SimpleFS : WEIRD CASES\n" ANSI_COLOR_RESET);
  printf("Here we test all those limit cases to check that the implementation works well even with big files/directories.\n");
  printf("In particular here we test the same functions in the simplefs_test but with directories and files contained in 3 blocks on disk.\n");

  // INIT THE DISK
  printf(ANSI_COLOR_CYAN "\nINIT THE DISK\n" ANSI_COLOR_RESET);
  DiskDriver dd;
  DiskDriver_init(&dd, "mysystem", TOTAL_BLOCKS); 
  SimpleFS fs; 
  DirectoryHandle* dhandle = SimpleFS_init(&fs, &dd);  
  printf("This is the dirhandle returned from init, it points to the root_directory\n");
  SimpleFS_printDirHandle(dhandle);


}