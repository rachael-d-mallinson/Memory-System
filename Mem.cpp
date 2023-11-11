//-----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#include "Mem.h"
#include "Heap.h"
#include "Block.h"

// To help with coalescing... not required
struct SecretPtr
{
	FreeHdr *pFree;
};

// ----------------------------------------------------
//  Initialized the Memory block:
//  Update the Heap
//  Create a free block
//  Heap structure links to free hdr
//-----------------------------------------------------
void Mem::initialize()
{
	Heap* pHeap = this->poHeap;

	//start and end of free header
	FreeHdr* pFreeStart = (FreeHdr*)(pHeap + 1);
	FreeHdr* pFreeEnd = pFreeStart + 1;
	//find total block size and pass in
	unsigned int BlockSize = (unsigned int)this->pBottomAddr - (unsigned int)pFreeEnd;

	FreeHdr* pFreeHdr = new(pHeap + 1) FreeHdr(BlockSize);

	//update Heap header
	pHeap->setFreeHead(pFreeHdr);
	pHeap->setNextFit(pFreeHdr);
	pHeap->setUsedHead(nullptr);

	//update the stats
	pHeap->AddFreeBlock(pFreeHdr->mBlockSize);

}

// ----------------------------------------------------
//  Do your Magic here:
//  Find a free block that fits
//  Change it to used (may require subdivision)
//  Correct the heap Links (used,free) headers 
//  Update stats
//  Return pointer to block
//-----------------------------------------------------
void *Mem::malloc( const uint32_t _size )
{
	
	//temp to walk list
	FreeHdr* currFreeH = this->GetHeap()->pNextFit;

	if (currFreeH == nullptr)
	{
		currFreeH = this->GetHeap()->pFreeHead;
	}
	
	//check to see if we cannot use it and if we can move on
	while (currFreeH->mBlockSize < (uint16_t)_size)
	{
		currFreeH = currFreeH->pFreeNext;
		if (currFreeH == nullptr)
		{
			currFreeH = this->GetHeap()->pFreeHead;
		}
		if (currFreeH == this->GetHeap()->pNextFit)
		{
			break;
		}
	}

	//if size needed is exact fit
	if (currFreeH->mBlockSize == (uint16_t)_size)
	{
		// take care of heap stats
		this->GetHeap()->currNumFreeBlocks--;
		this->GetHeap()->currFreeMem -= (uint16_t)_size;
		this->GetHeap()->IncUsedBlockPeakBlock();
		this->GetHeap()->IncMemAndPeakMem((uint16_t)_size);

		//update freehdr if it is the head of freehead list
		if (currFreeH == this->GetHeap()->pFreeHead)
		{
			this->GetHeap()->pFreeHead = currFreeH->pFreeNext;
			
		}
		//update freehdr pointers
		if (currFreeH->pFreePrev != nullptr)
		{
			currFreeH->pFreePrev->pFreeNext = currFreeH->pFreeNext;
		}
		//update freehdr pointers
		if (currFreeH->pFreeNext != nullptr)
		{
			currFreeH->pFreeNext->pFreePrev = currFreeH->pFreePrev;
		}

		this->GetHeap()->pNextFit = currFreeH->pFreeNext;


		//overwrite data with usedHdr
		currFreeH->mType = Block::Used;
		UsedHdr* newUsed = (UsedHdr*)currFreeH;
		newUsed->pUsedNext = nullptr;
		newUsed->pUsedPrev = nullptr;

		//move new used into used list
		insertUsedHdr(newUsed);

		FreeHdr* BlockEnd = (FreeHdr*)(((unsigned int)(newUsed + 1)) + (unsigned int)newUsed->mBlockSize);
		//correct the flag below
		if (BlockEnd != this->pBottomAddr)
		{
			BlockEnd->mAboveBlockFree = false;
		}


		return newUsed + 1;
	}

	//if the size needed is less than what is provided
	else if (currFreeH->mBlockSize > (uint16_t)_size)
	{
		splitBlock(currFreeH + 1, (uint16_t)_size);

		this->GetHeap()->currFreeMem -= ((uint16_t)_size + (int)sizeof(*currFreeH));//add the size of header

		this->GetHeap()->IncUsedBlockPeakBlock();
		this->GetHeap()->IncMemAndPeakMem((uint16_t)_size);

		//return the data block
		return currFreeH + 1;
	}

	else //we couldn't find any sufficient space
	{
		return nullptr;
	}
}

// ----------------------------------------------------
//  Do your Magic here:
//  Return the free block to the system
//  May require Coalescing
//  Correct the heap Links (used,free) headers 
//  Update stats
//-----------------------------------------------------
void Mem::free( void * const data )
{
	if (data == nullptr)
	{
		return;
	}

	//gives you the address of the hdr connected to data
	UsedHdr* currUsedData = (UsedHdr*)data;
	UsedHdr* currUsedH = currUsedData - 1;

	//update usedhdr if it is the head of head list, set it to the next
	if (currUsedH == this->GetHeap()->pUsedHead)
	{
		this->GetHeap()->pUsedHead = this->GetHeap()->pUsedHead->pUsedNext;
	}
	//update Usedhdr pointers
	if (currUsedH->pUsedPrev != nullptr)
	{
		currUsedH->pUsedPrev->pUsedNext = currUsedH->pUsedNext;
	}
	//update Usedhdr pointers
	if (currUsedH->pUsedNext != nullptr)
	{
		currUsedH->pUsedNext->pUsedPrev = currUsedH->pUsedPrev;
	}

	//overwrite with new data
	currUsedH->mType = Block::free;
	FreeHdr* currFreeH = (FreeHdr*)currUsedH;
	insertFreeHdr(currFreeH);

	//update heap stats
	this->GetHeap()->currUsedMem -= currFreeH->mBlockSize;
	this->GetHeap()->currNumUsedBlocks--;
	this->GetHeap()->currFreeMem += currFreeH->mBlockSize;
	this->GetHeap()->currNumFreeBlocks++;
	
	//put in the secret pointer
	unsigned int sizeToPointer = (unsigned int)currFreeH->mBlockSize - (unsigned int)sizeof(SecretPtr);
	SecretPtr* pSecretLocation = (SecretPtr*)((unsigned int)currUsedData + (unsigned int)sizeToPointer);
	SecretPtr* pSecret = new(pSecretLocation) SecretPtr;
	pSecret->pFree = currFreeH;
	

	//make sure we are not at the top of the heap
	if (currFreeH->mAboveBlockFree)
	{
		SecretPtr* pSecretBefore = (SecretPtr*)currFreeH;
		pSecretBefore = pSecretBefore - 1;

		if (pSecretBefore != nullptr)
		{
			if (pSecretBefore->pFree->mType == Block::free)
			{
				currFreeH = Coalesce(pSecretBefore->pFree, currFreeH);

				//update the secret pointer
				sizeToPointer = (unsigned int)currFreeH->mBlockSize - (unsigned int)sizeof(SecretPtr);
				FreeHdr* currFreeData = currFreeH + 1;
				pSecretLocation = (SecretPtr*)((unsigned int)currFreeData + (unsigned int)sizeToPointer);
				pSecret = new(pSecretLocation) SecretPtr;
				pSecret->pFree = currFreeH;
			}
		}
	}

	//check below block, coalecse if need
	if (pSecret + 1 != this->pBottomAddr)
	{
		FreeHdr* BlockEnd = (FreeHdr*)(pSecret + 1);

		if (BlockEnd->mType == Block::free)
		{
			currFreeH = Coalesce(currFreeH, BlockEnd);

			//update the secret pointer
			sizeToPointer = (unsigned int)currFreeH->mBlockSize - (unsigned int)sizeof(SecretPtr);
			FreeHdr* currFreeData = currFreeH + 1;
			pSecretLocation = (SecretPtr*)((unsigned int)currFreeData + (unsigned int)sizeToPointer);
			pSecret = new(pSecretLocation) SecretPtr;
			pSecret->pFree = currFreeH;

			if (pSecret + 1 != this->pBottomAddr)
			{
				BlockEnd = (FreeHdr*)(pSecret + 1);
				BlockEnd->mAboveBlockFree = true;
			}
		}
		else
		{
			BlockEnd->mAboveBlockFree = true;
		}
	}

	//check nextfit, reset if it got to end of heap
	if (this->GetHeap()->pNextFit == nullptr)
	{
		this->GetHeap()->pNextFit = this->GetHeap()->pFreeHead;
	}

}


//splits the given block into a used and free block, inserts in right lists
void Mem::splitBlock(FreeHdr* block, uint32_t _size)
{
	FreeHdr* oldBigFree = block - 1;
	uint32_t new_size = (oldBigFree->mBlockSize - ((unsigned int)_size + (int)sizeof(*oldBigFree)));

	bool isHead = false;
	if (oldBigFree == this->GetHeap()->pFreeHead) {isHead = true; }
	
	FreeHdr* toHoldPrev = oldBigFree->pFreePrev;
	FreeHdr* toHoldNext = oldBigFree->pFreeNext;

	//placement new a used header, with necessary block size
	UsedHdr* newUsedH = new(oldBigFree) UsedHdr(_size);
	newUsedH->mAboveBlockFree = oldBigFree->mAboveBlockFree;
	UsedHdr* newUsedEnd = newUsedH + 1;

	//get size of smaller free data
	//placement new a new FreeHdr after the used
	FreeHdr* currFreeH = new((FreeHdr*)((unsigned int)newUsedEnd + (unsigned int)_size)) FreeHdr(new_size);
	FreeHdr* currFreeEnd = currFreeH + 1;
	this->GetHeap()->pNextFit = currFreeH;

	//put in the secret pointer
	unsigned int sizeToPointer = (unsigned int)currFreeH->mBlockSize - (unsigned int)sizeof(SecretPtr);
	SecretPtr* pSecretLocation = (SecretPtr*)((unsigned int)currFreeEnd + (unsigned int)sizeToPointer);

	SecretPtr* pSecret = new(pSecretLocation) SecretPtr;
	pSecret->pFree = currFreeH;

	//update free header list - to skip around block because block is now split and used
	currFreeH->pFreePrev = toHoldPrev;
	currFreeH->pFreeNext = toHoldNext;
	if (isHead)
	{
		this->GetHeap()->pFreeHead = currFreeH;
		currFreeH->pFreeNext = this->GetHeap()->pFreeHead->pFreeNext;
	}
	if (currFreeH->pFreePrev != nullptr)
	{
		currFreeH->pFreePrev->pFreeNext = currFreeH;
	}
	if (currFreeH->pFreeNext != nullptr)
	{	
		currFreeH->pFreeNext->pFreePrev = currFreeH;
	}
	
	
	//insert used into front of used list
	insertUsedHdr(newUsedH);
}


//merges free boxes together that are next to each other
FreeHdr* Mem::Coalesce(void* _pFreeHead, void* _pFreeHead2)
{
	FreeHdr* currFreeH = (FreeHdr*)_pFreeHead;
	FreeHdr* blockToAdd = (FreeHdr*)_pFreeHead2;

	bool isHead = false;
	if (currFreeH == this->GetHeap()->pFreeHead) { isHead = true; }
	FreeHdr* toHoldPrev = currFreeH->pFreePrev;
	FreeHdr* toHoldNext = blockToAdd->pFreeNext;

	//create new block with size that combines both sizes, and the size of 1 header (12)
	unsigned int newSize = (unsigned int)currFreeH->mBlockSize + (unsigned int)blockToAdd->mBlockSize + (unsigned int)sizeof(*blockToAdd);
	FreeHdr* newFreeH = new (currFreeH) FreeHdr((uint16_t)newSize);

	//update some pointers
	newFreeH->pFreePrev = toHoldPrev;
	newFreeH->pFreeNext = toHoldNext;
	
	if (isHead)
	{
		this->GetHeap()->pFreeHead = newFreeH;
	}

	if (newFreeH->pFreePrev != nullptr)
	{
		newFreeH->pFreePrev->pFreeNext = newFreeH;
	}
	newFreeH->pFreeNext = blockToAdd->pFreeNext;
	if (newFreeH->pFreeNext != nullptr)
	{
		newFreeH->pFreeNext->pFreePrev = newFreeH;
	}

	//update header stats
	this->GetHeap()->currFreeMem += sizeof(*newFreeH); //just goes up by size of 1 header (12)
	this->GetHeap()->currNumFreeBlocks--;


	if (this->GetHeap()->pNextFit == currFreeH || this->GetHeap()->pNextFit == blockToAdd)
	{
		this->GetHeap()->pNextFit = newFreeH;
	}

	//check nextfit, reset if it got to end of heap
	if (this->GetHeap()->pNextFit == nullptr)
	{
		this->GetHeap()->pNextFit = this->GetHeap()->pFreeHead;
	}
	
	return newFreeH;
}


//inserts a used head into the front of a linked list of used heads
void Mem::insertUsedHdr(UsedHdr* newUsed)
{
	if (this->GetHeap()->pUsedHead == nullptr)
	{
		this->GetHeap()->pUsedHead = newUsed;
		newUsed->pUsedPrev = nullptr;
		newUsed->pUsedNext = nullptr;
	}
	else
	{
		this->GetHeap()->pUsedHead->pUsedPrev = newUsed;
		newUsed->pUsedNext = this->GetHeap()->pUsedHead;
		this->GetHeap()->pUsedHead = newUsed;
		newUsed->pUsedPrev = nullptr;
	}
}

//inserts a free box into the free list in sorted order
void Mem::insertFreeHdr(FreeHdr* _freeHdr)
{
	FreeHdr* currFreeH = this->GetHeap()->pFreeHead;

	if (this->GetHeap()->pFreeHead == nullptr)
	{
		this->GetHeap()->pFreeHead = _freeHdr;
		_freeHdr->pFreePrev = nullptr;
		_freeHdr->pFreeNext = nullptr;
		return;
	}

	if (_freeHdr < this->GetHeap()->pFreeHead)
	{
		_freeHdr->pFreeNext = this->GetHeap()->pFreeHead;
		this->GetHeap()->pFreeHead->pFreePrev = _freeHdr;
		this->GetHeap()->pFreeHead = _freeHdr;
		_freeHdr->pFreePrev = nullptr;
		return;
	}
	else
	{
		while (_freeHdr > currFreeH)
		{
			if (currFreeH->pFreeNext == nullptr)
			{
				currFreeH->pFreeNext = _freeHdr;
				_freeHdr->pFreePrev = currFreeH;
				_freeHdr->pFreeNext = nullptr;
				return;
			}
			else if (_freeHdr < currFreeH->pFreeNext)
			{
				_freeHdr->pFreeNext = currFreeH->pFreeNext;
				currFreeH->pFreeNext = _freeHdr;
				_freeHdr->pFreeNext->pFreePrev = _freeHdr;
				_freeHdr->pFreePrev = currFreeH;
			}
			currFreeH = currFreeH->pFreeNext;
		}
	}
}



// ---  End of File ---
