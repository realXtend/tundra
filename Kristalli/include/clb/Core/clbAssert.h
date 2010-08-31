/** @file clbAssert.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief assert() macro.

	Compilation units should include this instead of assert.h or cassert for 
	more robust invariant testing and consistency across different platforms.
*/
#include <cassert>

#ifndef clbAssert_h
#define clbAssert_h

#if 0
// Check that preprocessor definitions are consistent.. Allowed flag combinations:
// 1. Debug mode: _DEBUG
// 2. Development mode: _DEBUG and DEVEL
// 3. Profile mode: NDEBUG and PROFILING
// 4. Release mode: NDEBUG and RELEASE
#ifdef _DEBUG
	#if defined(NDEBUG) || defined(PROFILING) || defined(RELEASE)
		#error _DEBUG is mutually exclusive with NDEBUG, PROFILING and RELEASE!
	#endif
#endif
#ifdef DEVEL
	#if !defined(_DEBUG) || defined(NDEBUG) || defined(PROFILING) || defined(RELEASE)
		#error DEVEL is mutually exclusive with NDEBUG, PROFILING and RELEASE, and requires _DEBUG (Also, check that you're linking to Debug Runtime DLL's in Development mode!)
	#endif
#endif
#ifdef NDEBUG
	#if !defined(PROFILING) && !defined(RELEASE)
		#error Either PROFILING or RELEASE must be defined with NDEBUG!
	#endif
#endif
#ifdef PROFILING
	#if defined(RELEASE)
		#error PROFILING is mutually exclusive with RELEASE!
	#endif
#endif

#endif

/* Error levels:
	0 - Serious errors, ones that are meant to be present in final RELEASE builds.
	1 - Assertions that are enabled in internal DEVEL builds, but not in final versions.
	2 - Assertions that are possibly too expensive to check in the normal DEVEL builds, includes
	    low level checks (e.g. container sanity checks) and some serious invariant checking. This level is 
	    the most verbose error level, and is usually present in DEBUG mode only. */
#define ERRORLEVEL 0

// if CLBLIBTHROWEXCEPTIONS is defined, then the library uses c++ exceptions. If undefined, exit() is called.
#define CLBLIBTHROWEXCEPTIONS

/* if SILENTCRASHES is defined, actual error messages are suppressed and replaced with a generic 
	"Application error." message. This is usually used in final RELEASE builds where passing an error string
	to client is meaningless. */
// #define SILENTCRASHES

/* if DEBUGGERPRESENT is defined, then all errors are propagated to break to debugger. */
// #define DEBUGGERPRESENT


#ifdef WIN32
#include "clb_Win32/W32Assert.h"
#elif defined(SCEPSP)
//#define assert(x) { if (!(x)) AssertFailure(#x, __FILE__, __LINE__); }
#include "clb_PSP/PSPAssert.h"
#endif

#endif // clbAssert_h inclusion guard - deliberately end it here to have it possible to override a std assert() with one below.

#if 0

#undef assert
#ifndef RELEASE
#define assert(expr) \
{ \
	int assertFailed = 0; \
	(void)((!!(expr)) || (++assertFailed)); \
	if (assertFailed) \
	{ \
		LOGERROR("Expression \"%s\" failed in file %s, line %d.\n", #expr, __FILE__, __LINE__); \
		BREAKTODEBUGGER(); \
		THROW(#expr); \
	} \
}

#define assume(expr) \
{ \
	int assertFailed = 0; \
	(void)((!!(expr)) || (++assertFailed)); \
	if (assertFailed) \
	{ \
		LOGWARNING("Expression \"%s\" failed in file %s, line %d.\n", #expr, __FILE__, __LINE__); \
	} \
}

#else
#define assert(expr)
#define assume(expr)
#endif
/*
#define assert(expr) \
{ \
	if (!(expr)) \
	{ \
		LOGERROR("Expression \"%s\" failed in file %s, line %d.\n", #expr, __FILE__, __LINE__); \
		BREAKTODEBUGGER(); \
		THROW(#expr); \
	} \
}
*/
#undef FAIL
#define FAIL(msg, ...) \
{ \
	LOGERROR("Application error in file %s, line %d: \"" msg "\"\n", __FILE__, __LINE__, __VA_ARGS__); \
	BREAKTODEBUGGER(); \
	THROW(msg); \
} 

#undef REQUIRE
#define REQUIRE(expr, msg, ...) \
{ \
	if (!(expr)) \
	{ \
		LOGERROR("Expression \"%s\" failed in file %s, line %d: \"" msg "\"\n", #expr, __FILE__, __LINE__, __VA_ARGS__); \
		BREAKTODEBUGGER(); \
		THROW(msg); \
	} \
}

#define NOTIMPLEMENTED() LOGERROR("Unimplemented feature in file %s, line %d", __FILE__, __LINE__)


#endif