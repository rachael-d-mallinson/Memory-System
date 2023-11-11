//-----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#include "Used.h"
#include "Free.h"
#include "Block.h"

// Add magic here
FreeHdr::FreeHdr(unsigned int bSize)
{
    this->mAboveBlockFree = false;
    this->mType = Block::free;             // block type 
    this->mBlockSize = (uint16_t)bSize;            // size of block
    this->pFreeNext = nullptr;        // next free block
    this->pFreePrev = nullptr;
}

// ---  End of File ---
