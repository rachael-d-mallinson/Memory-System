//-----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#include "Heap.h"
#include "Mem.h"

Heap::Heap()
:	pUsedHead(nullptr),
	pFreeHead(nullptr),
	pNextFit(nullptr)
{
    peakNumUsed = 0;        // number of peak used allocations
    peakUsedMemory  = 0;    // peak size of used memory

    currNumUsedBlocks = 0;  // number of current used allocations
    currUsedMem = 0;        // current size of the total used memory

    currNumFreeBlocks = 0;  // number of current free blocks
    currFreeMem = 0;        // current size of the total free memory

    pad0 = 0;                // pad

}

void Heap::setFreeHead(FreeHdr* pFree)
{
    this->pFreeHead = pFree;
}
void Heap::setNextFit(FreeHdr* pNext)
{
    this->pNextFit = pNext;
}
void Heap::setUsedHead(UsedHdr* pUsed)
{
    this->pUsedHead = pUsed;
}

void Heap::AddFreeBlock(uint16_t BlockSize)
{
    //add a new block
    this->privAddFreeBlockCount();
    this->privAddFreeBlockSize(BlockSize);
}

void Heap::privAddFreeBlockCount()
{
    this->currNumFreeBlocks++;
}
void Heap::privAddFreeBlockSize(uint16_t BlockSize)
{
    this->currFreeMem += BlockSize;
}

//adds size to used memory and peak memory
void Heap::IncMemAndPeakMem(uint16_t _size)
{
    this->currUsedMem += _size;
    if (currUsedMem > peakUsedMemory)
    {
        this->peakUsedMemory = currUsedMem;
    }
}
//increments used blocks and peak used blocks
void Heap::IncUsedBlockPeakBlock()
{
    this->currNumUsedBlocks++;
    if (currNumUsedBlocks > peakNumUsed)
    {
        this->peakNumUsed = currNumUsedBlocks;
    }

}


// ---  End of File ---
