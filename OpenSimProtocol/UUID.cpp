// For conditions of distribution and use, see copyright notice in license.txt
#include <iomanip>
#include <iostream>
#include <sstream>

#include "UUID.h"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

/// Converts a single char to a value of 0-15. (4 bits)
static uint8_t CharToNibble(char c)
{
	if (isdigit(c))
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 0xA + c - 'a';
	if (c >= 'A' && c <= 'F')
		return 0xA + c - 'A';

	// Invalid octet.
	return 0xFF;
}

/// @return The first two characters of the given string converted to a byte: "B9" -> 0xB9.
static uint8_t StringToByte(const char *str)
{
	return (CharToNibble(str[0]) << 4) | CharToNibble(str[1]);
}

UUID::UUID()
{
	SetNull();
}

UUID::UUID(const char *str)
{
	if (str)
		FromString(str);
	else
		SetNull();
}

UUID::UUID(const std::string &str)
{
	FromString(str.c_str());
}

void UUID::SetNull()
{
	for(int i = 0; i < cSizeBytes; ++i)
		data[i] = 0;
}

/// Converts a C string representing a UUID to a uint8_t array.
/// Supports either the format "1c1bbda2-304b-4cbf-ba3f-75324b044c73" or "1c1bbda2304b4cbfba3f75324b044c73".
void UUID::FromString(const char *str)
{
	int curIndex = 0;
	for(int i = 0; i < cSizeBytes; ++i)
	{
		while(!(isalpha(str[curIndex]) || isdigit(str[curIndex]) || str[curIndex] == '\0')) 
			++curIndex;
			if (str[curIndex] == '\0')
				break;
		data[i] = StringToByte(str + curIndex);
		curIndex += 2;
	}
}

std::string UUID::ToString() const
{
	stringstream str;
	int i = 0;

	for(int j = 0; j < 4; ++j) str << hex << data[i++];
	str << "-";

	for(int j = 0; j < 2; ++j) str << hex << data[i++];
	str << "-";

	for(int j = 0; j < 2; ++j) str << hex << data[i++];
	str << "-";

	for(int j = 0; j < 2; ++j) str << hex << data[i++];
	str << "-";

	for(int j = 0; j < 6; ++j) str << hex << data[i++];

	return str.str();
}

bool UUID::operator ==(const UUID &rhs) const
{
	for(int i = 0; i < cSizeBytes; ++i)
		if (data[i] != rhs.data[i])
			return false;

	return true;
}

bool UUID::operator <(const UUID &rhs) const
{
	for(int i = 0; i < cSizeBytes; ++i)
		if (data[i] < rhs.data[i])
			return true;
		else if (data[i] > rhs.data[i])
			return false;

	return false;
}
