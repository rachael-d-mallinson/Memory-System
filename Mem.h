//-----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#ifndef MEM_H
#define MEM_H

#include "Heap.h"

class Mem
{
public:
	enum class Guard
	{
		Type_A,
		Type_5
	};

public:

	Mem( Guard type );	

	Mem() = delete;
	Mem(const Mem &) = delete;
	Mem & operator = (const Mem &) = delete;
	~Mem();

	Heap *GetHeap();
	void Print(int count);

	// implement these functions
	void free( void * const data );
	void *malloc( const uint32_t size );
	void initialize( );

	void splitBlock(FreeHdr* block, uint32_t _size);
	FreeHdr* Coalesce(void* _pFreeHead, void* pFreeHead2);

	void insertFreeHdr(FreeHdr* _freeHdr);
	void insertUsedHdr(UsedHdr* newUsed);

private:
	// Useful in malloc and free
	Heap *poHeap;
	void *pTopAddr;
	void *pBottomAddr;

	// Below: 
	//    Not used in malloc, free,or inialize
	Guard type;	
	void *poRawMem;
};

#endif 

// ---  End of File ---

