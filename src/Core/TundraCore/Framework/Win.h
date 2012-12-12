// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

// Windows.h is a bit tricky file to include, due to various reasons.
// Instead of including Windows.h directly, one should use this file, which
// removes a few of the annoying problems with it.

#ifdef WIN32

// Already included Windows.h by some other means. This is an error, since including winsock2.h will fail after this.
#if defined(_WINDOWS_) && !defined(_WINSOCK2API_) && defined(FD_CLR)
#error Error: Trying to include winsock2.h after windows.h! This is not allowed! See this file for fix instructions.
#endif

// Remove the manually added #define if it exists so that winsock2.h includes OK.
#if !defined(_WINSOCK2API_) && defined(_WINSOCKAPI_)
#undef _WINSOCKAPI_
#endif

// Windows.h issue: Cannot include winsock2.h after windows.h, so include it before.

// MathGeoLib uses the symbol Polygon. Windows.h gives GDI function Polygon, which Tundra will never use, so kill it.
#define Polygon Polygon_WINGDI_UNUSED

#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#undef Polygon

// Remove <Windows.h> PlaySound Unicode defines. Tundra does not use winmm PlaySound API anywhere (but OpenAL).
#ifdef PlaySound
#undef PlaySound
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#endif
