// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_StringUtil_h
#define incl_Core_StringUtil_h

namespace Core
{
    static std::wstring ToWString(const std::string &str)
    {
        std::wstring w_str(str.length(), L' ');
        std::copy(str.begin(), str.end(), w_str.begin());
        return w_str;
    }
}
 
#endif


