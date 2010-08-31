#ifndef DebugMemoryLeakCheck_h
#define DebugMemoryLeakCheck_h

#ifdef _DEBUG

#include <new>
#include <crtdbg.h>

#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
/*
__forceinline void *operator new(size_t size, const char *file, int line)
{
	return _malloc_dbg(size, _CLIENT_BLOCK, file, line);
}

__forceinline void *operator new[](size_t size, const char *file, int line)
{
	return _malloc_dbg(size, _CLIENT_BLOCK, file, line);
}

#define new new (__FILE__, __LINE__)
*/
#endif

#endif
