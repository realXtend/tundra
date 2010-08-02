/** @file W32Common.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage. No copying or redistribution may be performed without prior consent
	of the author(s). 

	@brief Declarations for Win32-specific global scope identifiers.
*/
#ifndef W32Common_h
#define W32Common_h

#include <string>

namespace clb
{
/// Windows API-specific functionality.
namespace win
{
/// Returns the last error string in the Win32 API.
std::string GetLastMSError();

/// Shows the last Win32 API error in a MessageBox.
void ShowLastMSError();
}

/// DirectX9.0c implementation.
namespace dx
{
}

}

#endif
