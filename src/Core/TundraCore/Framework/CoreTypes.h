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

#pragma warning "Not using Boost and C99 not defined. Guessing the built-ins for fixed-width types!"

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

#ifndef TUNDRA_NO_BOOST
using boost::shared_ptr;
using boost::weak_ptr;
using boost::dynamic_pointer_cast;
using boost::static_pointer_cast;
using boost::make_shared;
using boost::enable_shared_from_this;
using boost::regex;
using boost::wregex;
using boost::sregex_iterator;
using boost::regex_search;
using boost::smatch;
#else
using std::shared_ptr;
using std::weak_ptr;
using std::dynamic_pointer_cast;
using std::static_pointer_cast;
using std::make_shared;
using std::enable_shared_from_this;
using std::regex;
using std::wregex;
using std::sregex_iterator;
using std::regex_search;
using std::smatch;
#endif

typedef std::vector<std::string> StringVector;
typedef shared_ptr<StringVector> StringVectorPtr;

typedef std::list<std::string> StringList;
typedef shared_ptr<StringList> StringListPtr;
