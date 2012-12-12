/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   EnableMemoryLeakCheck.h
 *  @brief  (Re)enables a preprocessor macro that routes operator new to its debug counterpart. Win32 only.
 *  @sa     DisableMemoryLeakCheck.h
*/

#if defined(_MSC_VER) && defined(_DEBUG) && defined(MEMORY_LEAK_CHECK)
#ifdef new
#undef new
#endif
#define new new (__FILE__, __LINE__)
#endif
