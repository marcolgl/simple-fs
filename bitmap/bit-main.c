#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"ma

#define ANSI_COLOR_PURPLE  "\e[1;35m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_CYAN    "\e[1;36m"                
                

#define CHARS_IN_BITMAP 2
#define CHARS_IN_BITMAP2 40

int main(int argc, char** argv){


    printf(ANSI_COLOR_PURPLE "---BitMap: ALL BITMAP TESTS\n\n" ANSI_COLOR_RESET);    

    printf(ANSI_COLOR_CYAN "-TEST 1:" ANSI_COLOR_RESET "\nAllocate a 16 bit sized bitmap and set blocks 0,1,3,6,7,8,9.\n");
    BitMap* bmap = calloc(1,sizeof(BitMap));
    bmap->entries = calloc(CHARS_IN_BITMAP,sizeof(char));
    bmap->num_bits = CHARS_IN_BITMAP*8;    
    int set;    
    set = BitMap_set(bmap, 16, 1); // failed
    set = BitMap_set(bmap, 8, 1);
    set = BitMap_set(bmap, 2, 1);
    set = BitMap_set(bmap, 6, 1);
    set = BitMap_set(bmap, 7, 1);
    set = BitMap_set(bmap, 9, 1);
    set = BitMap_set(bmap, 3, 1);
    set = BitMap_set(bmap, 0, 1);
    set = BitMap_set(bmap, 1, 1);
    set = BitMap_set(bmap, 2, 0);
    BitMap_print(bmap);
    int res = BitMap_get(bmap, 0, 0);
    printf("\nFirst free block pos:%d\n",res);
    printf("Expected: 2\n\n");

    printf(ANSI_COLOR_CYAN "-TEST 2:" ANSI_COLOR_RESET "\nAllocate a 320 bit sized bitmap and set the first 230 blocks.\n");
    BitMap* bmap2 = calloc(1,sizeof(BitMap));
    bmap2->entries = calloc(CHARS_IN_BITMAP2,sizeof(char));
    bmap2->num_bits = CHARS_IN_BITMAP2*8;
    int i;
    for (i=0; i<230; i++)
        set = BitMap_set(bmap2, i, 1);
    BitMap_print(bmap2);
    res = BitMap_get(bmap2, 0, 0);
    printf("First free block pos: %d\n", res);
    printf("Expected: 230\n");
}
