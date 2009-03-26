// For conditions of distribution and use, see copyright notice in license.txt
#include "RexUUID.h"

using namespace std;

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

namespace RexTypes
{
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
    
    void RexUUID::Random()
    {
        for (int i = 0; i < cSizeBytes; ++i)
            data[i] = rand() & 0xff;
    }
    
    /// Converts a C string representing a RexUUID to a uint8_t array.
    /// Supports either the format "1c1bbda2-304b-4cbf-ba3f-75324b044c73" or "1c1bbda2304b4cbfba3f75324b044c73".
    void RexUUID::FromString(const char *str)
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
    
    std::string RexUUID::ToString() const
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
    
    bool RexUUID::operator ==(const RexUUID &rhs) const
    {
        for(int i = 0; i < cSizeBytes; ++i)
            if (data[i] != rhs.data[i])
                return false;
    
        return true;
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
}