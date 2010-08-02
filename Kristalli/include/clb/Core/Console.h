/** @file Console.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Console provides application output logging.
*/
#ifndef Console_h
#define Console_h

#include "ConsoleImpl.h"

namespace clb
{

/// Text output for logging.
class Console
{
private:
	ConsoleImpl platform;

	static const int numLines = 20;
	std::string lastLines[numLines];

public:
	Console();
	~Console();

	const char *LastLog(int lineNum) const;

	void Init(int dOutputFlags);	///< Opens the output streams.
	void DeInit();						///< Closes the output streams.

	void Write(const char *string);		///< Writes a string to the initialized outputs.
	void Write(const std::string &str);	///< Writes a string to the initialized outputs.
	void operator()(const char *string, ...);	///< Writes a string to the initialized outputs.
	void operator()(const std::string &str);	///< Writes a string to the initialized outputs.
};

namespace Global
{
	extern Console console;				///< Output logging subsystem.
};

}

#endif
