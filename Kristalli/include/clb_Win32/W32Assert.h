/** @file W32Assert.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
#define NOMINMAX
#include <windows.h>
//#include <DxErr.h>

//#include <cassert>
#include "clb_Win32/W32Common.h"
#include "clb_Win32/W32Exception.h"
#include "clb_Win32/W32Logging.h"

#ifdef DEBUGGERPRESENT
#define BREAKTODEBUGGER() _CrtDbgBreak()
#else
#define BREAKTODEBUGGER()
#endif

/*
First of all, the library can be built with CLBLIBEXCEPTIONS enabled or disabled. #define CLBLIBEXCEPTIONS
	will cause c++ exceptions to be used in all error reporting inside the library.
FAIL(msg):
	- In SILENTCRASHES mode: Halts execution by passing out a generic "Application Error!" message.
	- In DEBUG mode: immediately halts execution and prints out the given msg.
	- In RELEASE mode: immediately halts execution and prints out the given msg.

// REQUIRE halts execution if expr != true and outputs the message msg. Functions in DEBUG and RELEASE mode.
*/
#if defined(SILENTCRASHES)

//#define FAIL(msg) { THROW("Application Error!"); } 
//#define REQUIRE(expr, msg) { if (!(expr)) THROW("Application Error!"); }
#define W32TRY(hr, msg, ...) { if (FAILED(hr)) THROW("Application Error!"); }

#elif defined(_DEBUG)
/*
#define FAIL(msg) \
{ \
	LOGERROR("Application failure in file %s, line %d: \"%s\"\n", __FILE__, __LINE__, msg); \
	LOGERROR("GetLastError(): %s", clb::win::GetLastMSError().c_str()); \
	_CrtDbgBreak(); \
} 
#define REQUIRE(expr, msg, ...) \
{ \
	if (!(expr)) \
	{ \
		LOGERROR("Expression \"%s\" failed in file %s, line %d: \"%s\"\n", #expr, __FILE__, __LINE__, msg, __VA_ARGS__); \
		LOGERROR("GetLastError(): %s", clb::win::GetLastMSError().c_str()); \
		_CrtDbgBreak(); \
	} \
}*/
#define W32TRY(hr, msg, ...) \
{ \
	if (FAILED(hr)) \
	{ \
		const char *error = DXGetErrorString(hr); \
		LOGERROR("Error: \"%s\". Expression \"%s\" returned \"%s\" in file %s, line %d.\n", msg, __VA_ARGS__, #hr, error, __FILE__, __LINE__); \
		_CrtDbgBreak(); \
	} \
}

#elif defined(RELEASE)
/*
#define FAIL(msg) \
{ \
	LOGERROR("Application failure in file %s, line %d: \"%s\"\n", __FILE__, __LINE__, #msg); \
	THROW(#msg); \
} 
#define REQUIRE(expr, msg, ...) \
{ \
	if (!(expr)) \
	{ \
		LOGERROR("Expression \"%s\" failed in file %s, line %d: \"%s\"\n", #expr, __FILE__, __LINE__, msg, __VA_ARGS__); \
		THROW(msg); \
	} \
}*/
#define W32TRY(hr, msg, ...) \
{ \
	if (FAILED(hr)) \
	{ \
		LOGERROR(msg, __VA_ARGS__); \
		THROW(msg); \
	} \
}

#else
/*
#define FAIL(msg, ...) _CrtDbgBreak()//THROW(msg, __VA_ARGS__)
#define REQUIRE(expr, msg, ...) \
{ \
	if (!(expr)) \
	{ \
		LOGERROR(msg, __VA_ARGS__); \
		THROW(msg); \
	} \
}*/
#define W32TRY(hr, msg, ...) \
{ \
	if (FAILED(hr)) \
	{ \
		const char *error = DXGetErrorString(hr); \
		LOGERROR("Error: \"%s\". Expression \"%s\" returned \"%s\" in file %s, line %d.\n", msg, __VA_ARGS__, #hr, error, __FILE__, __LINE__); \
		THROW(error); \
	} \
}

#endif
