// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreStdIncludes_h__
#define incl_CoreStdIncludes_h__

#if defined(WIN32) || defined(WIN64)
#undef _WINDOWS
#define _WINDOWS
#endif

#if defined(WIN32) && !defined(_WIN32_WINNT)        // Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501                         // Change this to the appropriate value to target other versions of Windows.
#endif

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

#if defined(__APPLE__)
#include <Carbon/Carbon.h>
#ifdef check
#undef check /* mac <AssertMacros.h> conflict with boost exceptions */
#endif
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
#include <fstream>
#include <math.h>
#include <limits>

#include <boost/smart_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/timer.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

#include <Poco/Foundation.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/ClassLibrary.h>
#include <Poco/ClassLoader.h>
#include <Poco/Util/XMLConfiguration.h>
#include <Poco/XML/XMLWriter.h>
#include <Poco/Logger.h>

#include <Poco/Channel.h>
#include <Poco/Formatter.h>

//! Portable Component libraries. External library.
namespace Poco { }

//! Boost libraries. External library.
namespace boost { }


#endif

