#ifndef GLOBALALLOCOBJECT_H_
#define GLOBALALLOCOBJECT_H_

#include <cstdio>
#include <cstdlib>

template <typename T>
struct GlobalAllocObject
{   
   void * operator new(size_t sz )
   {  
#ifndef Q_WS_WIN
       return malloc(sz);
#else
       return GlobalAlloc(GPTR, sizeof(T)); 
#endif
   }

   void operator delete(void * ptr)
   {  
#ifndef Q_WS_WIN   
        free(ptr);
#else
       GlobalFree(ptr); 
#endif  

  }

   T* handle() { return (T*) this; }
};

#include "boost/smart_ptr.hpp"

template <typename T>
struct  SharedGlobalObject : 
          public boost::shared_ptr<GlobalAllocObject<T> > 
{
   typedef boost::shared_ptr<GlobalAllocObject<T> > sgPtr;

   SharedGlobalObject() : sgPtr (new sgPtr::element_type)  {}

   operator T*() { return get()->handle(); }
};

#endif // !defined(GLOBALALLOCOBJECT_H_INCLUDED_)
