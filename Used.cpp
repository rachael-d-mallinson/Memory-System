//-----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#include "Free.h"
#include "Used.h"

// Add code here
UsedHdr::UsedHdr(unsigned int _bSize)
{
    this->mBlockSize = (uint16_t)_bSize;
    this->mAboveBlockFree = false;
    this->mType = Block::Used;
    this->pUsedNext = nullptr;
    this->pUsedPrev = nullptr;
}


// ---  End of File ---

