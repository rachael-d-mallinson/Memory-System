//-----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#ifndef USED_H
#define USED_H

#include "Block.h"

class FreeHdr;

class UsedHdr
{
public:

    UsedHdr(unsigned int _bSize);

   // ---------------------------------------------------------
   // Do not reorder, change or add data fields
   //     --> any changes to the data... 0 given to the assignment
   // You can add methods if you wish
   // ---------------------------------------------------------

    bool       mAboveBlockFree;  // AboveBlock flag:
                                 // if(AboveBlock is type free) -> true 
                                 // if(AboveBlock is type used) -> false
    Block      mType;            // block type 
    uint16_t   mBlockSize;       // size of block (16 bit)
    UsedHdr   *pUsedNext;       // next used block
    UsedHdr   *pUsedPrev;       // prev used block

};

#endif 

// ---  End of File ---

