//-----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#ifndef FREE_H
#define FREE_H

#include "Block.h"

class UsedHdr;

class FreeHdr
{
public:

    //constructor
    FreeHdr(unsigned int bSize);

    // ---------------------------------------------------------
    // Do not reorder, change or add data fields
    //     --> any changes to the data... 0 given to the assignment
    // You can add methods if you wish
    // ---------------------------------------------------------

    bool     mAboveBlockFree;   // AboveBlock flag:
                                // if(AboveBlock is type free) -> true 
                                // if(AboveBlock is type used) -> false
    Block    mType;             // block type 
    uint16_t mBlockSize;        // size of block
    FreeHdr *pFreeNext;        // next free block
    FreeHdr *pFreePrev;        // prev free block

};

#endif 

// ---  End of File ---
