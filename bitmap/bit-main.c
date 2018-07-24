#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"ma
#define CHARS_IN_BITMAP 2

int main(int argc, char** argv){

    BitMap* bmap = calloc(1,sizeof(BitMap));
    bmap->entries = calloc(CHARS_IN_BITMAP,sizeof(char));
    bmap->num_bits = CHARS_IN_BITMAP*8;
    int set;// = BitMap_set(bmap, 3, 1);
    //set = BitMap_set(bmap, 5, 1);
    BitMap_print(bmap);
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
    printf("First free block pos:%d\n",res);

}
