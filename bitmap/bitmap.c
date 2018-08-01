#include "bitmap.h"
#include <stdio.h>



// converts a block index to an index in the array,
// and a char that indicates the offset of the bit inside the array
BitMapEntryKey BitMap_blockToIndex(int num){
    int entry_num = num/8;
    char bit_num = (char)(num%8);
    BitMapEntryKey res;
    res.entry_num = entry_num;
    res.bit_num = bit_num;
    return res;
}

// converts a bit to a linear index
int BitMap_indexToBlock(int entry, uint8_t bit_num){
    return entry*8+bit_num;
}


// sets the bit at index pos in bmap to status
int BitMap_set(BitMap* bmap, int pos, int status){
    if (pos >= bmap->num_bits || (status!=0 && status != 1))
        return -1;
    BitMapEntryKey bmek = BitMap_blockToIndex(pos);
    char mask =(char) 1 << bmek.bit_num;
    if (status == 1)
        bmap->entries[bmek.entry_num] |= mask;
    else
        bmap->entries[bmek.entry_num] &= ~mask;
    return 1;
}


// returns the index of the first bit having status "status"
// in the bitmap bmap, and starts looking from position start
int BitMap_get(BitMap* bmap, int start, int status){
    BitMapEntryKey bmek = BitMap_blockToIndex(start);
    char map;
    char mask = 1 << bmek.bit_num;
    char check = status << bmek.bit_num;
    //printf("starting bit_num = %d, starting entry_num = %d\n",bmek.bit_num, bmek.entry_num);
    while (BitMap_indexToBlock(bmek.entry_num, bmek.bit_num) < bmap->num_bits){
        //printf("index = %d\n",BitMap_indexToBlock(bmek.entry_num, bmek.bit_num));
        map = bmap->entries[bmek.entry_num];
        if ( !(map & mask ^ check) & mask != 0 )
            return BitMap_indexToBlock(bmek.entry_num, bmek.bit_num);
       // printf("control = %d, bit_num = %d, entry_num = %d, map = %d\n",mask & map , bmek.bit_num, bmek.entry_num, map);
        if (bmek.bit_num < 7 ){
            bmek.bit_num++;
            mask = mask << 1;
            check = check << 1;
        }
        else{
            bmek.bit_num = 0;
            mask = 1;
            check = status;
            bmek.entry_num++;
        }
    }
    return -1;
}


void BitMap_print(BitMap* bmap){
    int i;
    int prev_block_used = 0;
    int block_used;
    printf("\nBitmap representation:\n");
    for (i=0; i < bmap->num_bits; ){
        block_used = BitMap_get(bmap, prev_block_used ,1);
        if (block_used == -1){
            printf("0");
            i++;
        }
        else{
            while(prev_block_used < block_used){
                printf("0");
                prev_block_used++;
                i++;
            }
            printf("1");
            prev_block_used++;
            i++;
        }
    }
    printf("\nEnd of Bitmap\n");
}