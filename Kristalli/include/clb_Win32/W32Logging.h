/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/

#ifndef W32Logging_h
#define W32Logging_h

#pragma warning(disable:4996)

#include <iostream>

//#include "clb/Globals.h"
#include "clb/Core/Console.h"

//#ifdef DEBUGGERPRESENT

inline void OutputDebugStringVariadic(const char * /*filename*/, int /*lineNumber*/, const char *msg, ...)
{
	char errorStr[512];
	va_list args;
	va_start(args, msg);
	vsnprintf(errorStr, 512, msg, args);
//	char errorStr2[512];
//	sprintf(errorStr2, "%s:%d %s\n", filename, lineNumber, errorStr);

#ifdef DEBUGGERPRESENT
	OutputDebugString(errorStr);
	OutputDebugString("\n");
#endif
	std::cout << errorStr << std::endl;
//	clb::Global::console.Write(errorStr);

	va_end(args);
}

#define LOG(msg, ...) OutputDebugStringVariadic(__FILE__, __LINE__, msg, __VA_ARGS__)
#define LOGWARNING(msg, ...) OutputDebugStringVariadic(__FILE__, __LINE__, msg, __VA_ARGS__)
#define LOGERROR(msg, ...) OutputDebugStringVariadic(__FILE__, __LINE__, msg, __VA_ARGS__)
/*
#else

#define LOG(msg, ...) clb::Global::console(msg "\n", __VA_ARGS__)
#define LOGWARNING(msg, ...) clb::Global::console(msg "\n", __VA_ARGS__)
#define LOGERROR(msg, ...) clb::Global::console(msg "\n", __VA_ARGS__)

#endif
*/
#endif // inclusion guard
