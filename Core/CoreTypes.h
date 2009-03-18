// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreTypes_h
#define incl_CoreTypes_h

// types

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

    typedef unsigned int entity_id_t;
    typedef unsigned int event_category_id_t;
    typedef unsigned int event_id_t;

    typedef std::vector<std::string> StringVector;
    typedef boost::shared_ptr<StringVector> StringVectorPtr;

    typedef boost::lock_guard<boost::mutex> MutexLock;
    typedef boost::mutex Mutex;

    typedef boost::thread Thread;
}

#endif // incl_Types_h


