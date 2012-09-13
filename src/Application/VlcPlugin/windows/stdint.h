/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   stdint.h
    @brief  Needed for libvlc code when building with VC9. Do not include this file anywhere in Tundra code!
    @todo   It's very bad to have filename which conflicts with C99 standard files; consider findind a better workaround. */

#pragma once

#ifndef WIN32
#error This file should not be included in the build on non-Windows platforms!
#endif

#if (__STDC_VERSION__ < 199901L) || (_MSC_VER < 1600)

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;

#endif