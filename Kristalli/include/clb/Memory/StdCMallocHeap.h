/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
#ifndef StdCMallocHeap_h
#define StdCMallocHeap_h

#include <malloc.h>

#include "clb/Core/clbAssert.h"
#include "Allocator.h"
#include "clb/Algorithm/Alignment.h"

namespace clb
{
/** Allocates raw memory using the std C malloc and free. Note that the free/used -block tracking
	is left at the burden of the stdlib implementation, so don't rely on this allocator in performance
	critical sections. */
class StdCAlloc
{
public:
//	StdCAlloc(){}
//	StdCAlloc(const StdCAlloc &);
//	~StdCAlloc() {}

	static inline void *Alloc(StdCAlloc *, size_t size, size_t alignment, const char * /*nameTag*/ = 0, AllocFlags /*flags*/ = AFAllocLow)
	{
		assert(IS_POW2(alignment));
//		std::cout << "Allocating " << size << " bytes using the StdMallocHeap." << std::endl;
//		void *ptr = std::malloc(size);
		void *ptr = _aligned_malloc(size, alignment);
//		std::cout << "allocated ptr " << (int*)ptr << std::endl;
		return ptr;
	}
	static inline void Free(StdCAlloc *, void *ptr)
	{
//		std::cout << "freeing ptr at " << (int*)ptr << std::endl;
//		free(ptr);
		_aligned_free(ptr);
	}

	///\todo Perhaps support Resize(void *ptr, size_t newSize); ?
	static inline size_t Size(StdCAlloc *, void *ptr)
	{
		// I probably shouldn't even be calling this, since it's underscored.
		return ::_msize(ptr);
	}
};

}

#endif
