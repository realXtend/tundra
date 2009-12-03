// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreTypes_h
#define incl_CoreTypes_h

// types

#ifdef unix

#include <cmath>
#include <limits>
/**
 * Gnu GCC have C99-standard macros as an extension but in some system there does not exist them so we define them by ourself. 
 */
template <class T> inline bool _finite(T f) { return f != std::numeric_limits<T>::infinity(); }
template <class T> inline bool _isnan(T f) { return f != f; }

#endif 

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition.hpp>

namespace Core
{
    typedef unsigned char u8;
    typedef unsigned char uchar;
    typedef unsigned short u16;
    typedef unsigned int u32;
    typedef unsigned int uint;
    typedef unsigned long ulong;
    typedef short s16;
    typedef int s32;
    typedef float Real;
    typedef Real f32;
    typedef double f64;
    
    // etc... add more

    typedef int service_type_t;

    typedef unsigned int entity_id_t;
    typedef unsigned int event_category_id_t;
    typedef unsigned int event_id_t;

    namespace
    {
        event_category_id_t IllegalEventCategory = 0;
    }
    
    typedef unsigned int request_tag_t;
    typedef std::vector<request_tag_t> RequestTagVector;    

    typedef std::vector<std::string> StringVector;
    typedef boost::shared_ptr<StringVector> StringVectorPtr;

    typedef std::list<std::string> StringList;
    typedef boost::shared_ptr<StringList> StringListPtr;

    //! generic mutex
    typedef boost::mutex Mutex;
    //! recursive mutex
    typedef boost::recursive_mutex RecursiveMutex;
    //! Raii style mutex locking
    typedef boost::lock_guard<Mutex> MutexLock;
    //! Raii style recursive mutex locking
    typedef boost::lock_guard<RecursiveMutex> RecursiveMutexLock;
    //! Scoped lock
    typedef boost::mutex::scoped_lock ScopedLock;
    
    //! Thread
    typedef boost::thread Thread;
    //! Condition
    typedef boost::condition Condition;
}

#endif // incl_Types_h


