/** @file Console.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage. No copying or redistribution may be performed without prior consent
	of the author(s). 

	@brief Provides basic console for output
*/

#pragma once

#define NOMINMAX
#include <windows.h>
#include <string>
#include <sstream>

namespace clb
{
namespace win
{
// console initialization flags
#define CONOUT_SCREEN 1				///< Open the console to screen
#define CONOUT_FILE   2				///< Open the console to file "system.log"

/** @brief clb::Console implementation for Windows. Uses AllocConsole.

	Provides the application a console window for outputting text to console and file. */
class W32Console
{
private:
	/// Writes output to the specified handle.
	void WriteToHandle(HANDLE hOut, const char *string, int numChars = 0);

	/// Writes an 'End of Line' (0x0D 0x0A) character sequence to the buffer.
	void WriteEOL(HANDLE hOut);

public:
	W32Console() {}
//	~W32Console() {}

	void Init(int dOutputFlags);	///< Opens the output streams.
	void DeInit();						///< Closes the output streams.

	void Write(const char *string);
};

}
}
