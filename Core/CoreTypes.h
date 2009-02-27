// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_CoreTypes_h__
#define __incl_CoreTypes_h__

// types

namespace Core
{
    //typedef Ogre::Vector3 Vector3;
    //typedef Ogre::Vector4 Vector4;
    //typedef Ogre::Matrix4 Matrix4;
    typedef int Vector3;
    typedef int Vector4;
    typedef int Matrix3;
    typedef int Matrix4;
    typedef int Quaternion;

    typedef unsigned long ulong;
    typedef unsigned int uint;
    typedef unsigned int u32;
    typedef unsigned short u16;
    typedef unsigned char u8;
    typedef float Real;
    typedef unsigned char uchar;
    // etc... add more

    typedef unsigned int entity_id_t;

    typedef std::vector<std::string> StringVector;
    typedef boost::shared_ptr<StringVector> StringVectorPtr;

    typedef boost::lock_guard<boost::mutex> mutex_lock;
}

#endif // __incl_Types_h__


