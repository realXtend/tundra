
// THIS HEADER IS HERE ONLY FOR VLC INTERNALS THAT NEED STDINT.H.
// DO NOT INCLUDE THIS ANYWHERE IN TUNDRA CODE SO YOU WONT GET THE USING BOOST:: DEFINES.

#include "CoreTypes.h"

#if (__STDC_VERSION__ >= 199901L) || (_MSC_VER >= 1600)
    // Nothing do do
#else
    using boost::uint8_t;
    using boost::uint16_t;
    using boost::uint32_t;
    using boost::uint64_t;

    using boost::int8_t;
    using boost::int16_t;
    using boost::int32_t;
    using boost::int64_t;
#endif