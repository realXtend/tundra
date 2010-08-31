/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
#ifndef AllocTracker_h
#define AllocTracker_h

#include <iostream>

#include "clb/Core/clbAssert.h"
#include "clb/Memory/Allocator.h"
#include "clb/Container/FixedArray.h"
#include "clb/Algorithm/Sort.h"

namespace clb
{

struct AllocEntry
{
	void *ptr;
	size_t size;
	// size_t alignment;
	// AllocFlags flags;
	// string name;
	// u32 time;
	
	static int Cmp(const AllocEntry &a, const AllocEntry &b)
	{
		return clb::sort::TriCmp(a.ptr, b.ptr);
	}
};

template<typename Base, size_t MaxHandles>
class AllocTracker : public Base
{
private:
	FixedArray<AllocEntry, MaxHandles> entries;

public:
	AllocTracker() {}
	static inline void *Alloc(AllocTracker *allocator, size_t size, size_t alignment, const char *nameTag = 0, AllocFlags flags = AFAllocLow)
	{
		assert(allocator);
		void *ptr = Base::Alloc(allocator, size, alignment, nameTag, flags);
		if (!ptr)
			return 0;
		AllocEntry &e = allocator->entries.push_back();
		e.ptr = ptr;
		e.size = size;
		//entries[numEntriesInUse].alignment = alignment;
		//entries[numEntriesInUse].flags = flags;
		//entries[numEntriesInUse].name = nameTag;
		//entries[numEntriesInUse].time = timeCounter++;
		return ptr;
	}
	static inline void Free(AllocTracker *allocator, void *ptr)
	{
		assert(allocator);

		class CmpOp
		{
		private:
			void *ptr;
		public:
			CmpOp(void *ptr_):ptr(ptr_) {}
			inline bool operator()(const AllocEntry &e) const
			{
				// The second expression is a total hack to overcome VC CRT memory address 'shifts' when using array new.
				return e.ptr == ptr;// || (e.ptr < ptr && (intptr_t)e.ptr + 0x60 >= (intptr_t)ptr);
			}
		};
		size_t i = allocator->entries.find_index(CmpOp(ptr));
		//assert(i != entries.eNotFound);
		if (i != allocator->entries.eNotFound)
			allocator->entries.erase(i);
		Base::Free(allocator, ptr);
	}
	///\todo Perhaps support Resize(void *ptr, size_t newSize); ?
	inline size_t Size(AllocTracker *allocator, void *ptr)
	{
		return Base::Size(allocator, ptr);
	}

	void PrintAllocationEntries()
	{
		size_t totalUsed = 0;
		size_t freeInBetween = 0;
		clb::sort::QuickSort(entries.begin(), entries.size(), AllocEntry::Cmp);
//		assert(clb::sort::IsSorted(entries.begin(), entries.size(), AllocEntry::Cmp));
		for(size_t i = 0; i < entries.size(); ++i)
		{
			// Warning: LOG can't allocate any memory here.
			LOG("0x%X: %d Bytes", entries[i].ptr, entries[i].size);
			totalUsed += entries[i].size;
			if (i+1 != entries.size())
			{
				intptr_t endAddress = (intptr_t)entries[i].ptr + entries[i].size;
//				assert(entries[i].ptr < entries[i+1].ptr);
//				assert(endAddress <= (intptr_t)entries[i+1].ptr);
				size_t gap = (intptr_t)entries[i+1].ptr - endAddress;
				freeInBetween += gap;
				LOG("0x%X: Free (%d Bytes)", endAddress, gap);
			}
//			std::clog << "0x" << entries[i].ptr << ": " << entries[i].size << " Bytes" << std::endl;
//			LOG("0x%X: %d Bytes %s", entries[i].ptr, entries[i].size, entries[i].name);
		}
		LOG("Total used: %d Bytes, Total lost due to fragmetation: %d Bytes",
			totalUsed, freeInBetween);
	}
};

}

#endif
