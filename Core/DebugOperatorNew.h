/** @file MemoryLeakCheck.h
    For conditions of distribution and use, see copyright notice in license.txt

    @brief Debug functionality for tracking memory leaks on the Win32 platform.

    Include this file in a .cpp right after including StableHeaders.h file to enable the C++ operator new to 
    route to log-enabled debug version of malloc in the current compilation unit. Do not include 
    this in any .h files.

    After getting the list of leaks when you close the application, look for entries saying 
    "client block" and "Unknown source". Unfortunately this method can not tell you the exact file/line of
    these allocations - to do that, see MemoryLeakCheck.h
*/

#ifndef incl_Core_DebugOperatorNew_h
#define incl_Core_DebugOperatorNew_h

#if defined(_MSC_VER) && defined(_DEBUG) 

#define _CRTDBG_MAP_ALLOC

__forceinline void *operator new(std::size_t size)
{
    return _malloc_dbg(size, _CLIENT_BLOCK, "Unknown source", 1);
}

__forceinline void *operator new[](std::size_t size)
{
    return _malloc_dbg(size, _CLIENT_BLOCK, "Unknown source[]", 1);
}

__forceinline void operator delete(void *ptr)
{
    _free_dbg(ptr, _CLIENT_BLOCK);
}

__forceinline void operator delete[](void *ptr)
{
    _free_dbg(ptr, _CLIENT_BLOCK);
}

#endif // ~_MSC_VER

#endif // ~incl_Core_DebugOperatorNew_h
