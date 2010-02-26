// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   RexUUID.cpp
 *  @brief  RexUUID is a 16-byte identifier for resources in a virtual world.
*/

#include "RexUUID.h"

#include <iomanip>
#include <cstring>

using namespace std;

/// Converts a single char to a value of 0-15. (4 bits)
namespace
{

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
/// Has no error checking. Just returns 0xFF if the parsing failed.
static uint8_t StringToByte(const char *str)
{
    return (CharToNibble(str[0]) << 4) | CharToNibble(str[1]);
}

}

RexUUID::RexUUID()
{
    SetNull();
}

RexUUID::RexUUID(const char *str)
{
    if (str)
        FromString(str);
    else
        SetNull();
}

RexUUID::RexUUID(const std::string &str)
{
    FromString(str.c_str());
}

void RexUUID::SetNull()
{
    for(int i = 0; i < cSizeBytes; ++i)
        data[i] = 0;
}

bool RexUUID::IsNull() const
{
    for(int i = 0; i < cSizeBytes; ++i)
        if (data[i] != 0)
            return false;
    return true;
}

void RexUUID::Random()
{
    for(int i = 0; i < cSizeBytes; ++i)
        data[i] = rand() & 0xff;
}

RexUUID RexUUID::CreateRandom()
{
    RexUUID id;
    id.Random();
    return id;
}

bool RexUUID::IsValid(const char *str)
{
    ///\bug Crashes if the uuid is valid otherwise but is too long and has unvalid chars at the very end.
    if (!str)
        return false;

    int valid_nibbles = 0;

    while (*str)
    {
        // If it looks anything like a url, can't be UUID
        if ((*str == '/') || (*str == ':'))
            return false;

        if (CharToNibble(*str) <= 0xf)
            valid_nibbles++;
        str++;
    }

    return (valid_nibbles == cSizeBytes * 2);
}

/// Converts a C string representing a RexUUID to a uint8_t array.
/// Supports either the format "1c1bbda2-304b-4cbf-ba3f-75324b044c73" or "1c1bbda2304b4cbfba3f75324b044c73".
/// If the inputted string is zero, or if the length is zero, or if a parsing error occurs, the UUID will be
/// set to null.
void RexUUID::FromString(const char *str)
{
    const int strLen = (str == 0) ? 0 : strlen(str);
    if (strLen == 0)
    {
        SetNull();
        return;
    }
    int curIndex = 0;
    for(int i = 0; i < cSizeBytes; ++i)
    {
        if (curIndex >= strLen)
        {
            SetNull();
            return;
        }

        ///\bug Tighten parsing, now accepts all characters, like 'g' or 'Y'.
        while(!isalpha(str[curIndex]) && !isdigit(str[curIndex]) && !str[curIndex] == '\0')
            ++curIndex;

        // The following parse needs to read two characters from the string, hence the +1.
        if (curIndex + 1 >= strLen)
        {
            SetNull();
            return;
        }

        ///\bug Here, if str[curIndex+1] is not an appropriate character, there's going to be an error!
        data[i] = StringToByte(str + curIndex);
        curIndex += 2;
    }
}

std::string RexUUID::ToString() const
{
    stringstream str;
    int i = 0;

    for(int j = 0; j < 4; ++j) str << hex << setw(2) << setfill('0') << (int)data[i++];
    str << "-";

    for(int j = 0; j < 2; ++j) str << hex << setw(2) << setfill('0') << (int)data[i++];
    str << "-";

    for(int j = 0; j < 2; ++j) str << hex << setw(2) << setfill('0') << (int)data[i++];
    str << "-";

    for(int j = 0; j < 2; ++j) str << hex << setw(2) << setfill('0') << (int)data[i++];
    str << "-";

    for(int j = 0; j < 6; ++j) str << hex << setw(2) << setfill('0') << (int)data[i++];

    return str.str();
}

RexUUID &RexUUID::operator =(const RexUUID &rhs)
{
    if (this != &rhs)
        for(int i = 0; i < cSizeBytes; ++i)
            data[i] = rhs.data[i];
     return *this;
}

bool RexUUID::operator ==(const RexUUID &rhs) const
{
    for(int i = 0; i < cSizeBytes; ++i)
        if (data[i] != rhs.data[i])
            return false;

    return true;
}

bool RexUUID::operator !=(const RexUUID &rhs) const
{
    for(int i = 0; i < cSizeBytes; ++i)
        if (data[i] != rhs.data[i])
            return true;

    return false;
}

bool RexUUID::operator <(const RexUUID &rhs) const
{
    for(int i = 0; i < cSizeBytes; ++i)
        if (data[i] < rhs.data[i])
            return true;
        else if (data[i] > rhs.data[i])
            return false;

    return false;
}

