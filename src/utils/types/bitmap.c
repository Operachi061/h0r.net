#include "bitmap.h"
#include <utils/logging/logger.h>

//Bit Magic: dont touch unless you have 2000000 IQ
//it took me half an hour in godbolt.org to figure out

void bitmap_set(struct Bitmap bm,size_t index,bool setto)
{
    register size_t byteIndex = index / 8;
    register uint8_t bitIndex = index % 8;

    register uint8_t bitMask = (1 << (bitIndex));

    register uint8_t temp = bm.buffer[byteIndex] ^ (bm.buffer[byteIndex] & bitMask);

    bm.buffer[byteIndex] = temp | (setto << (bitIndex));
}

bool bitmap_get(struct Bitmap bm,size_t index)
{
    register size_t byteIndex  = index / 8;
    register uint8_t bitIndex   = index % 8;

    register uint8_t bitIndexer = 1 >> bitIndex;
    return bm.buffer[byteIndex] & bitIndexer;
}