/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
#ifndef clb_Heap_h
#define clb_Heap_h

#include "clb/Core/clbAssert.h"
#include "clb/Memory/Allocator.h"
#include "clb/Algorithm/Alignment.h"
#include "clb/Container/FixedArray.h"

namespace clb
{
/** Manages arbitrary allocations on the given memory area. The memory used to track the allocations is statically
	allocated in the heap itself. Note that depending on the value of MaxHandles, the size of the structure can be 
	quite big, so might want to avoid creating the Heap structure on the stack.

	Heap doesn't read or write to anywhere in the memory address it is managing. */
template<size_t MaxHandles>
class Heap
{
private:
	/// Describes an used contiguous memory block inside the heap.
	struct AllocEntry
	{
		/// Starting address.
		uintptr_t ptr;
		u32 size; ///< in bytes.

		/// Returns the first memory address that is NOT part of the block, i.e. the first byte after this block.
		void *EndAddress() const { return (void*)(ptr + size); }
	};
	/// Stores all the allocation entries, always sorted in ascending order by the value of ptr.
	FixedArray<AllocEntry, MaxHandles> entries;

	// Parameters that describe the heap location.
	uintptr_t heapStart;
	u32 heapSize;

	/// Allocates the given amount of data from the heap, prefers the lowest available address.
	/// @return The allocated ptr, or 0 if allocation failed.
	/// \todo Save the nameTag to AllocEntry if memory allocation tagging is enabled.
	inline void *AllocLow(u32 size, u32 alignment, const char * /*nameTag*/)
	{
		// alignedStart: The aligned starting address of a free space block.
		// freeEnd: The ending address of a free space block - the first address that's not part of the free space block.

		// Look at the beginning of the heap: (== end of the heap if the heap is empty)
		uintptr_t alignedStart = AlignUpPow2(heapStart, alignment);
		if (alignedStart + size > heapStart + heapSize) // The allocation can't fit even if the heap was empty?
			return 0;
		uintptr_t freeEnd = (entries.size() == 0) ? heapStart + heapSize : entries[0].ptr;
		if (alignedStart + size <= freeEnd) // Do we have enough free space at the beginning?
		{
			AllocEntry &e = entries.insert(0); // Insert the new entry at the beginning, index 0. (Need to keep entries sorted)
			e.ptr = alignedStart;
			e.size = size;
			return (void*)e.ptr;
		}

		// Look at all the gaps:
		for(size_t i = 0; i+1 < entries.size(); ++i)
		{
			alignedStart = AlignUpPow2(entries[i].ptr + entries[i].size, alignment);
			freeEnd = entries[i+1].ptr;
			if (alignedStart + size <= freeEnd) // Found a gap that's big enough?
			{
				AllocEntry &e = entries.insert(i+1); // Insert the new entry between i and i+1. (Need to keep entries sorted)
				e.ptr = alignedStart;
				e.size = size;
				return (void*)e.ptr;
			}
		}

		// Look at the end:
		alignedStart = AlignUpPow2(entries[entries.size()-1].ptr + entries[entries.size()-1].size, alignment);
		freeEnd = heapStart + heapSize;
		if (alignedStart + size <= freeEnd)
		{
			AllocEntry &e = entries.push_back(); // Insert the new entry at the back. (Need to keep entries sorted)
			e.ptr = alignedStart;
			e.size = size;
			return (void*)e.ptr;
		}

		// Couldn't accommodate.
		return 0;
	}

public:
	/// Creates an uninitialized heap.
	Heap():heapStart(0), heapSize(0) {}

	/// Creates an initialized heap.
	Heap(void *startAddress, u32 size)
	{
		Init(startAddress, size);
	}

	/// Asserts if there are any previous allocation entries.
	void Init(void *startAddress, u32 size)
	{
		heapStart = (uintptr_t)startAddress;
		heapSize = size;

		assert(entries.size() == 0);
		entries.clear(); // Ditch any previous data entries.
	}

	/// Asserts if there are any previous allocation entries.
	void DeInit()
	{
		assert(entries.size() == 0);
		entries.clear(); // Ditch any previous data entries.
		heapStart = 0;
		heapSize = 0;
	}

	/// @param flags Allocation flags, @see AllocFlags.
	/// @return A pointer to allocated space, or 0 if allocation failed.
	inline void *Alloc(u32 size, u32 alignment, AllocFlags flags = AFAllocLow, const char *nameTag = 0)
	{
		if ((flags & AFAllocLow) != 0)
			return AllocLow(size, alignment, nameTag);
		else // Must be (flags & allocHigh) != 0
		{
			THROW("BadAlloc");
			return 0;///\todo: return AllocHigh(size, alignment, nameTag);
		}
	}

	/// Removes the given memory block from the list of used entries. Pass in the starting address of the block.
	/// It is safe to pass a null pointer. Asserts if the given pointer was not found in this heap.
	inline void Free(void *ptr)
	{
		if (!ptr)
			return;

		for(size_t i = 0; i < entries.size(); ++i)
			if (entries[i].ptr == (uintptr_t)ptr)
			{
				entries.erase(i);
				return;
			}

		FAIL("Pointer 0x%X was not allocated in this heap!", ptr);
	}

	/// @return The total size of the heap in bytes.
	inline u32 TotalSize() const { return heapSize; }

	/// @return First used address.
	inline void *StartAddress() const { return (void*)heapStart; }

	/// @return The first memory address after this heap.
	inline void *EndAddress() const { return (void*)(heapStart + heapSize); }

	/// @return The total amount of memory free in the eap.
	inline u32 BytesFree() const
	{
		if (entries.size() == 0)
			return TotalSize(); // Totally empty.

		u32 free = 0;
		// Free at start:
		free += (u32)(entries[0].ptr - heapStart);
		// Free at end:
		free += (u32)((uintptr_t)EndAddress() - (uintptr_t)entries[entries.size()-1].EndAddress());
		// Free in between:
		for(size_t i = 0; i + 1 < entries.size(); ++i)
			free += (u32)(entries[i+1].ptr - (uintptr_t)entries[i].EndAddress());

		return free;
	}

	/// @return The size of the given memory block. Pass in the starting address of the block.
	///         Returns 0 on a null pointer or if the pointer was not allocated in this heap.
	inline u32 Size(void *ptr) const
	{
		if (!ptr)
			return 0;

		for(size_t i = 0; i < entries.size(); ++i)
			if (entries[i].ptr == (uintptr_t)ptr)
				return entries[i].size;

		return 0;
	}

	///\todo Perhaps support bool Resize(void *ptr, size_t newSize); for potentially resizing an existing block?
};

} // ~clb

#endif
