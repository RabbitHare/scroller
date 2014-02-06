#ifndef __POOL_ALLOCATOR_H__
#define __POOL_ALLOCATOR_H__
#include <stddef.h>
template<class T, int blockSize>
class PoolAllocator
{
private:
	struct Block;

	struct Element
	{
		union
		{
			Element* next; // Used when in a block's free list.

			// Used when freeing this element to find
			// the block it is a member of.
			Block* block;
		}ptr;
		char t[sizeof(T)]; // Array to store the object.
	};
	struct Block
	{
		Block *prev,
		      *next;
		Element* free;
		int numFree;
		Element elements[blockSize];
	};
public:
	PoolAllocator();
	~PoolAllocator();

	T* 	Alloc(); // Allocates but does not construct Objects.

	void	Free(T* t); // the Object are returned to pool. If the number of
			    // of free elements exceeds a set amount, and the
			    // block from which this object was returned has
			    // all its elements free, the block is deleted.

	void	Purge(); // Deletes all blocks.
	
	inline int GetNumBlocks(){ return mNumBlocks; }
private:

	Block *mFreeBlocks,
	      *mFullBlocks;
	int mNumBlocks,
	    mNumFree;
};
/*
============
  PollAllocator()
============
*/
template<class T, int blockSize>
PoolAllocator<T,blockSize>::PoolAllocator()
{
	mFreeBlocks = mFullBlocks = NULL;
	mNumFree = mNumBlocks = 0;
}
/*
============
  ~PollAllocator()
============
*/
template<class T, int blockSize>
PoolAllocator<T,blockSize>::~PoolAllocator()
{
	Purge();
}
/*
============
  Alloc()
============
*/
template<class T, int blockSize>
T* PoolAllocator<T,blockSize>::Alloc()
{
	if (mFreeBlocks == NULL)
	{
		//create new block of elements
		Block* block = new Block();

		block->prev = NULL;
		block->free = NULL;
		block->numFree = blockSize;
		mNumFree += blockSize;

		block->next = mFreeBlocks;
		mFreeBlocks = block;
		//link elements
		for ( int i = 0; i < blockSize; ++i)
		{
			block->elements[i].ptr.next = block->free;
			block->free = &block->elements[i];
		}
		mNumBlocks++;
	}

	Element* element = mFreeBlocks->free;
	mFreeBlocks->free = mFreeBlocks->free->ptr.next;
	// Keep track of the block this element is a part of.
	element->ptr.block = mFreeBlocks;	
	
	mNumFree--;
	mFreeBlocks->numFree--;
	if (mFreeBlocks->numFree == 0)	
	{
		Block* next = mFreeBlocks->next;
		// Add to full list.
		mFreeBlocks->next = mFullBlocks;
		if (mFullBlocks)
			mFullBlocks->prev = mFreeBlocks;
		mFullBlocks = mFreeBlocks;

		// Remove from free list.
		mFreeBlocks = next;
		if (mFreeBlocks)
			mFreeBlocks->prev = NULL;
	}

	return reinterpret_cast< T* >( &element->t );
}
/*
============
  Free()
============
*/
template<class T, int blockSize>
void PoolAllocator<T,blockSize>::Free( T* t)
{
	if (t == NULL) return;

	Element* element = (Element*)( ((unsigned char *)t ) - (unsigned char*)(&(((Element *)0)->t)));

	Block* block = element->ptr.block;
	element->ptr.next = block->free;
	block->free = element;
	if (block->numFree == 0) // Was full, add to free list.
	{
		// Remove from full list.
		if (block->prev)
			block->prev->next = block->next;
		else mFullBlocks = block->next;
		if (block->next)
			block->next->prev = block->prev;
		// Add to free list.
		block->next = mFreeBlocks;
		if (mFreeBlocks)
			mFreeBlocks->prev = block;
		block->prev = NULL;
		mFreeBlocks = block;
	}
	block->numFree++;
	mNumFree++;
	if (block->numFree == blockSize && mNumFree > blockSize*2-1)
	{
	// delete the empty block
		if (block->prev)
			block->prev->next = block->next;
		else mFreeBlocks = block->next;
		if (block->next)
			block->next->prev = block->prev;
		delete block;
		mNumFree -= blockSize;
		mNumBlocks--;
	}


}
/*
============
  Purge()
============
*/
template<class T, int blockSize>
void PoolAllocator<T,blockSize>::Purge()
{
	Block *blocks,
	      *block;

	// delete free blocks
	blocks = mFreeBlocks;
	while (blocks)
	{
		block = blocks;
		blocks = blocks->next;
		delete block;
	}

	// delete full blocks
	blocks = mFullBlocks;
	while (blocks)
	{
		block = blocks;
		blocks = blocks->next;
		delete block;
	}
	mFreeBlocks = mFullBlocks = NULL;
	mNumBlocks = mNumFree = 0;
}
#endif //!__POLL_ALLOCATOR_H__
