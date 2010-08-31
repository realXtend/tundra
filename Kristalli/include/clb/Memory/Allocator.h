/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	All allocators in the system are instance-based (in contrast to class-based). 
	This implies a few things:
		1. Because all memory allocation is performed through allocators, you need to create an allocator 
		   instance to be able to allocate memory.
		2. Each container will have a pointer to the allocator instance it's using.
		3. There are no general global (static/class-based/compile time -based) memory allocators.

	See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1850.pdf for loosely related discussion
	(mainly class-based vs instance-based allocator system).
*/
#ifndef Allocator_h
#define Allocator_h

#include "clb/Core/Types.h"

namespace clb
{

typedef u32 AllocFlags;
enum
{
	AFAllocLow  = 0x1,
	AFAllocHigh = 0x2
};

/** The global memory/object allocation function. Allocates sizeof(Object) bytes of memory using the allocator
	and uses placement new to construct an object of type Object to that memory area. (calls the ctor of Object).
	@return A pointer to the allocated object or 0 if failed.
	\todo Currently bad_alloc exceptions are not meant to be supported (although they can occur). Define this
		behavior more precisely? */
template<typename Object, typename Allocator>
inline Object *New(Allocator *allocator, size_t alignment, const char *nameTag = 0, AllocFlags flags = AFAllocLow)
{
//	REQUIRE(allocator, "Null allocator in New!");
	void *mem = Allocator::Alloc(allocator, sizeof(Object), alignment, nameTag);
	return new(mem) Object;
}

/** Allocates an array of objects. Calls default ctors for each.
	\todo The 4 bytes of size data are unnecessary for PODs! */
template<typename Object, typename Allocator>
inline Object *NewArray(size_t numObjects, Allocator *allocator, size_t alignment, const char *nameTag = 0,
								AllocFlags flags = AFAllocLow)
{
//	REQUIRE(allocator, "Null allocator in NewArray!");

	unsigned long extraSize = sizeof(size_t);

	///\todo Proper alignment.
	void *ptr = Allocator::Alloc(allocator, sizeof(Object)*numObjects + extraSize, alignment, nameTag, flags);
	*reinterpret_cast<size_t*>(ptr) = numObjects;

	Object *mem = reinterpret_cast<Object*>(reinterpret_cast<size_t*>(ptr)+1);
	for(size_t i = 0; i < numObjects; ++i)
		new (&mem[i]) Object;

	return mem;
}

/** Object deletion function. Destroys the given object (calls dtor) and returns the used memory back to the
	allocator. */
template<typename Object, typename Allocator>
inline void Delete(Object *ptr, Allocator *allocator)
{
	if (!ptr)
		return;
//	REQUIRE(allocator, "Null allocator in Delete!");

	ptr->~Object();
	Allocator::Free(allocator, ptr);
}

/** Array of objects deletion function. Destroys all objects (calls dtors for each) and returns the used memory
	back to the allocator. */
template<typename Object, typename Allocator>
inline void DeleteArray(Object *ptr, Allocator *allocator)
{
	if (!ptr)
		return;

	size_t numObjects = *(reinterpret_cast<size_t*>(ptr)-1);
	if (numObjects == 0)
		return;

//	REQUIRE(allocator, "Null allocator in DeleteArray!");

	// The standard mandates that we need to tear down the objects in the reverse order they were allocated in.
	for(size_t i = numObjects; i > 0; --i)
		ptr[i-1].~Object();

	Allocator::Free(allocator, reinterpret_cast<size_t*>(ptr)-1);
}

} // ~clb

#endif
