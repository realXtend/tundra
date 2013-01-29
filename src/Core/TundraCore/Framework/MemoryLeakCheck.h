/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file MemoryLeakCheck.h
    @brief Debug functionality for tracking memory leaks on the Win32 platform.

    Include this file in a .cpp file to enable C++ operator new to route to log-enabled debug version 
    of malloc. The best method is to include this after all other includes in a .cpp file (or 
    preferably right before including any template/allocating .inl's in the file) Do not include 
    this in any .h files. The reason why this is not included globally is that it wouldn't compile. 
    It relies on a really dirty hack of redefining operator new. Know that it cannot necessarily 
    track all the invokations of new, due to scope issues.

    After getting the list of leaks when you close the application, look for entries saying 
    "client block" */

#pragma once

#if defined(_MSC_VER) && defined(_DEBUG) && defined(MEMORY_LEAK_CHECK)

#include <ios>
#include <new>
#include <map>

#define _CRTDBG_MAP_ALLOC

#ifdef new
#undef new
#endif

// For the difference between _NORMAL_BLOCK and _CLIENT_BLOCK, see DebugOperatorNew.h

__forceinline void *operator new(std::size_t size, const char *file, int line)
{
    return _malloc_dbg(size, _NORMAL_BLOCK, file, line);
}

__forceinline void *operator new[](std::size_t size, const char *file, int line)
{
    return _malloc_dbg(size, _NORMAL_BLOCK, file, line);
}

__forceinline void operator delete(void *ptr, const char *, int)
{
    _free_dbg(ptr, _NORMAL_BLOCK);
}

__forceinline void operator delete[](void *ptr, const char *, int)
{
    _free_dbg(ptr, _NORMAL_BLOCK);
}

#define new new (__FILE__, __LINE__)

#endif // ~_MSC_VER
