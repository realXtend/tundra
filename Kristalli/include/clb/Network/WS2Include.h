/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Use this file to include winsock2.h. This file fixes the issue where winsock2.h has to be included before windows.h.
*/
#pragma once

/* The source of this error comes from the fact that if <windows.h> is included before <winsock2.h>, <windows.h> will include
	<winsock.h>, which is the older version of the winsock library. After that, if the user includes winsock2.h, he will get
	compilation errors.

	To fix this problem, do one of the following:

	1) Alter the inclusion order in the compilation unit(s) so that you explicitly #include <winsock2.h> before including
	   windows.h.
	2) Add a #define _WINSOCKAPI_ either to project settings, or at the top of the offending compilation unit (before including 
	   windows.h). Use this solution when you don't want to alter the inclusion order to the proper desired order (winsock2.h 
	   before windows.h)
*/

#if defined(_WINDOWS_) && !defined(_WINSOCK2API_) && defined(FD_CLR)
#error Error: Trying to include winsock2.h after windows.h! This is not allowed! See this file for fix instructions.
#endif

// Remove the manually added #define if it exists so that winsock2.h includes OK.
#if !defined(_WINSOCK2API_) && defined(_WINSOCKAPI_)
#undef _WINSOCKAPI_
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
