//-----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#ifndef HEAPHDR_H
#define HEAPHDR_H

#include "Used.h"
#include "Free.h"

class Heap
{
public:
	// Big four
	Heap();
	Heap(const Heap&) = delete;
	Heap& operator =(const Heap&) = delete;
	~Heap() = default;

	//just stupid wrappers
	void setFreeHead(FreeHdr* pFree);
	void setNextFit(FreeHdr* pNextFit);
	void setUsedHead(UsedHdr* pUsed);

	void AddFreeBlock(uint16_t BlockSize);


	void IncMemAndPeakMem(uint16_t _size);
	void IncUsedBlockPeakBlock();

	
	
	//void swapBlocks(FreeHdr *freeBlock, uint32_t size);

private:
	void privAddFreeBlockCount();
	void privAddFreeBlockSize(uint16_t BlockSize);




public:
	// Verify that the Heap is 16 byte aligned.

	// allocation links
	UsedHdr	*pUsedHead;
	FreeHdr	*pFreeHead;

	// Next fit allocation strategy
	FreeHdr    *pNextFit;

	uint16_t    currNumUsedBlocks;   // number of current used allocations
	uint16_t    currUsedMem;         // current size of the total used memory

	uint16_t    peakNumUsed;         // number of peak used allocations
	uint16_t    peakUsedMemory;      // peak size of used memory

	uint16_t    currNumFreeBlocks;   // number of current free blocks
	uint16_t    currFreeMem;         // current size of the total free memory

	uint32_t    pad0;

	
};

#endif 

// ---  End of File ---
