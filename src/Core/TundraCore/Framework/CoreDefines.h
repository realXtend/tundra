// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <cassert>

#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#define SAFE_DELETE(p) { delete p; p=0; }
#define SAFE_DELETE_ARRAY(p) { delete [] p; p=0; }

#define NUMELEMS(x) (sizeof(x)/sizeof(x[0]))

/// Use this template to downcast from a base class to a derived class when you know by static code analysis what the derived 
/// type has to be and don't want to pay the runtime performance incurred by dynamic_casting. In debug mode, the proper
/// derived type will be assert()ed, but in release mode this be just the same as using static_cast.
/// Repeating to make a note: In RELEASE mode, checked_static_cast == static_cast. It is *NOT* a substitute to use in places
/// where you really need a dynamic_cast.
template<typename Dst, typename Src>
inline Dst checked_static_cast(Src src)
{
    assert(src == 0 || dynamic_cast<Dst>(src) != 0);
    return static_cast<Dst>(src);
}

/** @def UNUSED_PARAM(x)
    Preprocessor macro for suppressing unused formal parameter warnings while still showing the variable name in Doxygen documentation.  */
#if defined(DOXYGEN) // DOXYGEN is a special define used when Doxygen is run.
#define UNUSED_PARAM(x) x
#else
#define UNUSED_PARAM(x)
#endif

/// Use to suppress warning C4101 (unreferenced local variable)
#define UNREFERENCED_PARAM(P) (void)(P);

/// Use for QObjects
#define SAFE_DELETE_LATER(p) { if ((p)) (p)->deleteLater(); (p) = 0; }

/** @def DEPRECATED(func)
    Raises compiler warning for a deprecated function. */
#ifdef __GNUC__
#define DEPRECATED(func) func __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#else
#define DEPRECATED(func) func
#endif
