// For conditions of distribution and use, see copyright notice in license.txt

#ifndef ProtocolUtilities_Md5_h
#define ProtocolUtilities_Md5_h

#include <string>

namespace ProtocolUtilities
{
    /// @return Hexadecimal MD5 hash string of the given string.
    std::string GetMd5Hash(const std::string &str);
}
#endif
