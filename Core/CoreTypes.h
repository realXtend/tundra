// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_CoreTypes_h
#define incl_Core_CoreTypes_h

// types

#if defined(unix) || defined(__APPLE__)

#include <cmath>
#include <limits>
/**
 * Gnu GCC have C99-standard macros as an extension but in some system there does not exist them so we define them by ourself. 
 */
template <class T> inline bool _finite(T f) { return f != std::numeric_limits<T>::infinity(); }
template <class T> inline bool _isnan(T f) { return f != f; }

#endif 

#include <boost/shared_ptr.hpp>

#include <vector>
#include <list>

#include <QString>
#include <QVector>
#include <QStringList>

#include <boost/cstdint.hpp>

typedef boost::uint8_t u8; ///< a single byte: 0-255.
typedef boost::uint16_t u16; ///< 2 bytes: 0 - 65535.
typedef boost::uint32_t u32; ///< 4 bytes: 0 - 4,294,967,295 ~ 4000 million or 4e9.
typedef boost::uint64_t u64; ///< 8 bytes: 18,446,744,073,709,551,615 ~1.8e19.

typedef boost::int8_t s8; ///< a single byte: -128 - 127.
typedef boost::int16_t s16; ///< 2 bytes: -32768 - 32767.
typedef boost::int32_t s32; ///< 4 bytes signed: max 2,147,483,647 ~ 2000 million or 2e9.
typedef boost::int64_t s64; ///< 8 bytes signed. 9,223,372,036,854,775,807 ~ 9e18.

typedef float f32;
typedef double f64;

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef int service_type_t;

typedef unsigned int entity_id_t;
typedef unsigned int event_category_id_t;
typedef unsigned int event_id_t;
typedef unsigned int sound_id_t;

namespace
{
    event_category_id_t IllegalEventCategory = 0;
}

typedef unsigned int request_tag_t;
typedef std::vector<request_tag_t> RequestTagVector;

typedef std::vector<std::string> StringVector;
typedef boost::shared_ptr<StringVector> StringVectorPtr;

typedef QVector<QString> QStringVector;

typedef std::list<std::string> StringList;
typedef boost::shared_ptr<StringList> StringListPtr;

#endif // incl_Types_h


