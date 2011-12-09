/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   DisableMemoryLeakCheck.h
 *  @brief  Clears the #define new macro that is used for debug memory leak checking.
 *  
 *  #include this to overcome situations in code where placement new or other problematic construction is
 *  used that interferes with the #define new in MemoryLeakCheck.h.
 *  
 *  #include EnableMemoryLeakCheck.h afterwards to re-enable memory leak checking for the remainder of
 *  the compilation unit.
 *  
 *  @sa     EnableMemoryLeakCheck.h
*/

#if defined(_MSC_VER) && defined(_DEBUG) && defined(MEMORY_LEAK_CHECK)
#undef new
#endif
