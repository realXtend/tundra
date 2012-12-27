/** @file DebugOperatorNew.h
    For conditions of distribution and use, see copyright notice in LICENSE

    @brief Debug functionality for tracking memory leaks on the Win32 platform.

    Include this file in a .cpp right after including StableHeaders.h file to enable the C++ operator new to 
    route to log-enabled debug version of malloc in the current compilation unit. Do not include 
    this in any .h files.

    After getting the list of leaks when you close the application, look for entries saying 
    "client block" and "Unknown source". Unfortunately this method can not tell you the exact file/line of
    these allocations - to do that, see MemoryLeakCheck.h
*/

#pragma once

#if defined(_MSC_VER) && defined(_DEBUG) && defined(MEMORY_LEAK_CHECK)
#include <utility>
#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>

#define _CRTDBG_MAP_ALLOC

// Ideally, we would like to allocate _CLIENT_BLOCKs in all the handlers below,
// but this has caused an assertion failure in crtdbg, where a memory block allocated
// by our code (as a _CLIENT_BLOCK) gets freed as if it were a _NORMAL_BLOCK. So,
// currently allocate all our blocks as _NORMAL_BLOCKs as well, which can make grepping
// a bit more difficult.

__forceinline void *operator new(std::size_t size)
{
	return _malloc_dbg(size, _NORMAL_BLOCK, DEBUG_CPP_NAME, 1);
}

__forceinline void *operator new[](std::size_t size)
{
	return _malloc_dbg(size, _NORMAL_BLOCK, DEBUG_CPP_NAME ", op[]", 1);
}

__forceinline void operator delete(void *ptr)
{
    _free_dbg(ptr, _NORMAL_BLOCK);
}

__forceinline void operator delete[](void *ptr)
{
    _free_dbg(ptr, _NORMAL_BLOCK);
}

#endif // ~_MSC_VER
