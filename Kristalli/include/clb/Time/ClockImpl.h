/** @file ClockImpl.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
#ifdef WIN32

#include "clb_Win32/W32Clock.h"
namespace clb
{
	typedef win::W32Clock ClockImpl;
}

#elif defined(SCEPSP)

#include "clb_PSP/PSPClock.h"
namespace clb
{
typedef psp::PSPClock ClockImpl;
}

#endif
