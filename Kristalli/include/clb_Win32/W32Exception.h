/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/

#ifndef W32Exception_h
#define W32Exception_h

//#include "clb/Win32/W32Logging.h"
/*
#ifdef _DEBUG
#include <crtdbg.h>
#endif
/*
#ifdef CLBLIBEXCEPTIONS

#include <exception>
#define THROW(msg) throw std::exception(msg);

#else // CLBLIBEXCEPTIONS not defined
#ifdef _DEBUG
#define THROW(msg) \
{ \
	LOGERROR(msg); \
	_CrtDbgBreak(); \
}
#else // _DEBUG not defined
#define THROW(msg, ...) MessageBox(0, msg, "Application quit", MB_OK)
#endif // _DEBUG

#endif // CLBLIBEXCEPTIONS
*/
#define CLBLIBEXCEPTIONS
#ifdef CLBLIBEXCEPTIONS

#include <exception>
#define THROW(msg) throw std::exception(msg);

#else // CLBLIBEXCEPTIONS not defined

#define THROW(msg) exit(-1); ///\todo Check that functionality is as wanted.

#endif


#endif // W32Exception_h
