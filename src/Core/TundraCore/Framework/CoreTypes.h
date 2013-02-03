// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

// types

#include <vector>
#include <list>

#include <QString>
#include <QStringList>

#if defined(unix) || defined(__APPLE__)
#include <cmath>
#include <limits>
// Gnu GCC has C99-standard macros as an extension but in some system there does not exist them so we define them by ourself.
template <class T> inline bool _finite(const T &f) { return f != std::numeric_limits<T>::infinity(); }
template <class T> inline bool _isnan(const T &f) { return f != f; }
#endif

// If we have C99, take the types from there.
#if (__STDC_VERSION__ >= 199901L) || (_MSC_VER >= 1600)

#include <cstdint>

typedef uint8_t u8; ///< a single byte: 0-255.
typedef uint16_t u16; ///< 2 bytes: 0 - 65535.
typedef uint32_t u32; ///< 4 bytes: 0 - 4,294,967,295 ~ 4000 million or 4e9.
typedef uint64_t u64; ///< 8 bytes: 18,446,744,073,709,551,615 ~1.8e19.

typedef int8_t s8; ///< a single byte: -128 - 127.
typedef int16_t s16; ///< 2 bytes: -32768 - 32767.
typedef int32_t s32; ///< 4 bytes signed: max 2,147,483,647 ~ 2000 million or 2e9.
typedef int64_t s64; ///< 8 bytes signed. 9,223,372,036,854,775,807 ~ 9e18.

// Otherwise, if we have Boost and its usage is not disabled, we can also pull the types from there.
#elif !defined(TUNDRA_NO_BOOST)

#include <boost/cstdint.hpp>

typedef boost::uint8_t u8; ///< a single byte: 0-255.
typedef boost::uint16_t u16; ///< 2 bytes: 0 - 65535.
typedef boost::uint32_t u32; ///< 4 bytes: 0 - 4,294,967,295 ~ 4000 million or 4e9.
typedef boost::uint64_t u64; ///< 8 bytes: 18,446,744,073,709,551,615 ~1.8e19.

typedef boost::int8_t s8; ///< a single byte: -128 - 127.
typedef boost::int16_t s16; ///< 2 bytes: -32768 - 32767.
typedef boost::int32_t s32; ///< 4 bytes signed: max 2,147,483,647 ~ 2000 million or 2e9.
typedef boost::int64_t s64; ///< 8 bytes signed. 9,223,372,036,854,775,807 ~ 9e18.

#else // No Boost or unknown if we have C99. Have to guess the following are correct.

#include <limits.h>

//#pragma warning "Not using Boost and C99 not defined. Guessing the built-ins for fixed-width types!"

typedef unsigned char u8; ///< a single byte: 0-255.
typedef unsigned short u16; ///< 2 bytes: 0 - 65535.
typedef unsigned long long u64; ///< 8 bytes: 18,446,744,073,709,551,615 ~1.8e19.

typedef signed char s8; ///< a single byte: -128 - 127.
typedef signed short s16; ///< 2 bytes: -32768 - 32767.

#if ULONG_MAX == 0xffffffff
typedef unsigned long u32; ///< 4 bytes: 0 - 4,294,967,295 ~ 4000 million or 4e9.
typedef long s32; ///< 4 bytes signed: max 2,147,483,647 ~ 2000 million or 2e9.
#elif UINT_MAX == 0xffffffff
typedef unsigned int u32; ///< 4 bytes: 0 - 4,294,967,295 ~ 4000 million or 4e9.
typedef int s32; ///< 4 bytes signed: max 2,147,483,647 ~ 2000 million or 2e9.
#endif

typedef signed long long s64; ///< 8 bytes signed. 9,223,372,036,854,775,807 ~ 9e18.

#endif

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
#else
#include <memory>
#include <regex>
#endif

/** @def CORETYPES_NAMESPACE
    The namespace from which C++ TR1 functionalities are used (boost, std::tr1 or std). */
#ifndef TUNDRA_NO_BOOST
#define CORETYPES_NAMESPACE boost
#elif defined(_MSC_VER) && (_MSC_VER == 1500) /**< @todo Find out which GCC version has the TR1 features Tundra uses. */
#define CORETYPES_NAMESPACE std::tr1
#elif defined(_MSC_VER) && (_MSC_VER >= 1600) /**< @todo Find out which GCC version has TR1 functionality moved to std namespace. */
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

/** @def MAKE_SHARED(type, ...)
    Workaround for the fact that make_shared is not a C++ TR1 feature, but a C++11 feature.
    If make_shared is not available, the shared_ptr is constructed using the old-fashioned way.
    @todo Although make_shared is a nice little optimization, consider if this macro workaround
    is worth the trouble and worth keeping. */
#if !defined(TUNDRA_NO_BOOST) || (defined(_MSC_VER) && (_MSC_VER >= 1600)) /**< @todo Find out which GCC version has make_shared. */
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
#if defined(TUNDRA_NO_BOOST) && defined(_MSC_VER) && (_MSC_VER == 1500)
#define WEAK_PTR_LESS_THAN(a, b) a._Cmp(b)
#else
#define WEAK_PTR_LESS_THAN(a, b) a.owner_before(b)
#endif

typedef std::vector<std::string> StringVector;
typedef shared_ptr<StringVector> StringVectorPtr;

typedef std::list<std::string> StringList;
typedef shared_ptr<StringList> StringListPtr;
