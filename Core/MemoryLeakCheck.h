/** @file MemoryLeakCheck.h
    For conditions of distribution and use, see copyright notice in license.txt

    @brief Debug functionality for tracking memory leaks on the Win32 platform.

    Include this file in a .cpp file to enable C++ operator new to route to log-enabled debug version 
    of malloc. The best method is to include this after all other includes in a .cpp file (or 
    preferably right before including any template/allocating .inl's in the file) Do not include 
    this in any .h files. The reason why this is not included globally is that it wouldn't compile. 
    It relies on a really dirty hack of redefining operator new. Know that it cannot necessarily 
    track all the invokations of new, due to scope issues.

    After getting the list of leaks when you close the application, look for entries saying 
    "client block"
*/
#ifndef incl_Core_MemoryLeakCheck_h
#define incl_Core_MemoryLeakCheck_h

#if defined(_MSC_VER) && defined(_DEBUG) 

#include <new>

#define _CRTDBG_MAP_ALLOC

#ifdef new
#undef new
#endif

__forceinline void *operator new(std::size_t size, const char *file, int line)
{
    return _malloc_dbg(size, _NORMAL_BLOCK, file, line);
//    return _malloc_dbg(size, _CLIENT_BLOCK, file, line);
}

__forceinline void *operator new[](std::size_t size, const char *file, int line)
{
    return _malloc_dbg(size, _NORMAL_BLOCK, file, line);
//    return _malloc_dbg(size, _CLIENT_BLOCK, file, line);
}

__forceinline void operator delete(void *ptr, const char *, int)
{
    _free_dbg(ptr, _NORMAL_BLOCK);
//    _free_dbg(ptr, _CLIENT_BLOCK);
}

__forceinline void operator delete[](void *ptr, const char *, int)
{
    _free_dbg(ptr, _NORMAL_BLOCK);
//    _free_dbg(ptr, _CLIENT_BLOCK);
}

#define new new (__FILE__, __LINE__)

#endif // ~_MSC_VER

#endif // ~incl_Core_MemoryLeakCheck_h
