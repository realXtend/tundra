// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_CoreStdIncludes_h__
#define __incl_CoreStdIncludes_h__

#if defined(WIN32) || defined(WIN64)
#undef _WINDOWS
#define _WINDOWS
#endif

#if defined(_WINDOWS) && !defined(_WIN32_WINNT)		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	                        // Change this to the appropriate value to target other versions of Windows.
#endif

#if defined(_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <queue>

#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/thread.hpp>

#include <Poco/Foundation.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/ClassLibrary.h>
#include <Poco/ClassLoader.h>
#include <Poco/Util/XMLConfiguration.h>


#endif

