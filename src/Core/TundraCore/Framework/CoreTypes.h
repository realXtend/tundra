// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

// types

#include <vector>
#include <list>

#include <QString>
#include <QStringList>
#include <QMetaType>

#if defined(unix) || defined(__APPLE__)
#include <cmath>
#include <limits>
// Gnu GCC has C99-standard macros as an extension but in some system there does not exist them so we define them by ourself.
template <class T> inline bool _finite(const T &f) { return f != std::numeric_limits<T>::infinity(); }
template <class T> inline bool _isnan(const T &f) { return f != f; }
#endif

// Use kNet/Types.h for fixed-width types instead of duplicating the code here.
#include <kNet/Types.h>

// Floating-point types
typedef float f32;
typedef double f64;

// Unsigned types
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;

// Special Tundra identifiers
typedef unsigned int entity_id_t;
typedef unsigned int component_id_t;

#ifndef TUNDRA_NO_BOOST
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>
#else
#include <memory>
#include <regex>
#include <unordered_map>
#endif

/** @def CORETYPES_NAMESPACE
    The namespace from which C++ TR1 functionalities are used (boost, std::tr1 or std).
    The following symbols must be used without the namespace:
    -shared_ptr, weak_ptr, dynamic_pointer_cast, static_pointer_cast, enable_shared_from_this,
    -regex, wregex, sregex_iterator, regex_search, smatch, wsmatch,
    -unordered_map and unordered_multimap.

    @note As of Xcode 4.X Apple has dropped GCC in favor of clang, which means that
          a 'stock GCC' in Xcode is very unlikely in the future. Last used GCC version in
          Macs was 4.2 which has initial TR1 implementations. For now, we assume that 
          Mac developers will use stock compilers. If one thinks otherwise, feel free to make
          the nessessary tests and changes here. 
          This has not been tested yet on Linux, at least to my knowledge. -cs */
#ifndef TUNDRA_NO_BOOST
#define CORETYPES_NAMESPACE boost
#elif defined(Q_OS_LINUX)
#define CORETYPES_NAMESPACE std
#elif defined(_MSC_VER) && (_MSC_VER == 1500) || (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ >= 2 && !defined(__APPLE__))
#define CORETYPES_NAMESPACE std::tr1
#elif defined(_MSC_VER) && (_MSC_VER >= 1600) || (defined(__APPLE__) && defined(__clang__) && __clang_major__ == 4 && __clang_minor__ == 2) || (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ >= 8 && !defined(__APPLE__))
#define CORETYPES_NAMESPACE std
#endif

// From <memory>:
using CORETYPES_NAMESPACE::shared_ptr;
using CORETYPES_NAMESPACE::weak_ptr;
using CORETYPES_NAMESPACE::dynamic_pointer_cast;
using CORETYPES_NAMESPACE::static_pointer_cast;
using CORETYPES_NAMESPACE::enable_shared_from_this;
// From <regex>:
using CORETYPES_NAMESPACE::regex;
using CORETYPES_NAMESPACE::wregex;
using CORETYPES_NAMESPACE::sregex_iterator;
using CORETYPES_NAMESPACE::regex_search;
using CORETYPES_NAMESPACE::smatch;
using CORETYPES_NAMESPACE::wsmatch;
// From <unordered_map>:
using CORETYPES_NAMESPACE::unordered_map;
using CORETYPES_NAMESPACE::unordered_multimap;

/** @def MAKE_SHARED(type, ...)
    Workaround for the fact that make_shared is not a C++ TR1 feature, but a C++11 feature.
    If make_shared is not available, the shared_ptr is constructed using the old-fashioned way.
    @todo Although make_shared is a nice little optimization, consider if this macro workaround
    is worth the trouble and worth keeping. */
#if !defined(TUNDRA_NO_BOOST) || (defined(_MSC_VER) && (_MSC_VER >= 1600)) || defined(Q_OS_LINUX) || (defined(__APPLE__) && defined(__clang__) && __clang_major__ == 4 && __clang_minor__ == 2)
#define MAKE_SHARED(type, ...) CORETYPES_NAMESPACE::make_shared<type>(__VA_ARGS__)
#else
#define MAKE_SHARED(type, ...) shared_ptr<type>(new type(__VA_ARGS__))
#endif

/** @def WEAK_PTR_LESS_THAN(a, b)
    In Tundra codebase ownership-based ordering is used for weak_ptr's less-than operator as this has been the default behavior
    due to boost::weak_ptr's implementation. This macro works around the fact that VC9SP1's std::tr1::weak_ptr doesn't have
    owner_before function, but mysteriously named _Cmp instead. _Cmp performs also ownership-based comparison so it's compliant
    with the original use cases.
    Recommended reading regarding this topic: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2004/n1590.html */
#ifdef TUNDRA_NO_BOOST
    #if defined(_MSC_VER) && (_MSC_VER == 1500)
        #define WEAK_PTR_LESS_THAN(a, b) a._Cmp(b)
    #else
        #define WEAK_PTR_LESS_THAN(a, b) a.owner_before(b)
    #endif
#else
    #define WEAK_PTR_LESS_THAN(a, b) a < b
#endif

/// Utility struct for providing less than operator for C++ standard weak_ptr.
template <typename T>
struct WeakPtrLessThan
{
    bool operator() (const weak_ptr<T> &a, const weak_ptr<T> &b) const { return WEAK_PTR_LESS_THAN(a, b); }
};

typedef std::vector<std::string> StringVector;
typedef shared_ptr<StringVector> StringVectorPtr;

typedef std::list<std::string> StringList;
typedef shared_ptr<StringList> StringListPtr;

Q_DECLARE_METATYPE(QList<QObject*>)
Q_DECLARE_METATYPE(std::string)
