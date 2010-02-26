// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_StringUtil_h
#define incl_Core_StringUtil_h

#include <iomanip>

#include <Poco/LocalDateTime.h>
#include <boost/lexical_cast.hpp>

#include "CoreDefines.h"
#include "CoreTypes.h"

static std::wstring ToWString(const std::string &str)
{
    std::wstring w_str(str.length(), L' ');
    std::copy(str.begin(), str.end(), w_str.begin());
    return w_str;
}

//! Converts value to a string. May throw boost::bad_lexical_cast.
template <class T>
static std::string ToString(const T &val) { return boost::lexical_cast<std::string>(val); }

//! Converts string to a primitive type, such as int or float. May throw boost::bad_lexical_cast.
template <typename T>
static T ParseString(const std::string &val) { return boost::lexical_cast<T>(val); }

//! Converts string to a primitive type, such as int or float. Returns default value on boost::bad_lexical_cast
template <typename T>
static T ParseString(const std::string &val, T default_value) 
{ 
    try
    {
        return boost::lexical_cast<T>(val); 
    }
    catch (boost::bad_lexical_cast e)
    {
        return default_value;
    }
}

//! Get the current time as a string.
static std::string GetLocalTimeString()
{
    Poco::LocalDateTime *time = new Poco::LocalDateTime(); 
    std::stringstream ss;
    
    ss << std::setw(2) << time->hour() << std::setfill('0') << ":" <<
        std::setw(2) << time->minute() << std::setfill('0') << ":" <<
        std::setw(2) << time->second() << std::setfill('0');
        
    SAFE_DELETE(time);
    
    return ss.str();
}

//! Get the current date and time as a string.
static std::string GetLocalDateTimeString()
{
    Poco::LocalDateTime *time = new Poco::LocalDateTime();
    std::stringstream ss;
    
    ss << std::setw(2) << time->day() << std::setfill('0') << "/" <<
        std::setw(2) << time->month() << std::setfill('0') << "/" <<
        std::setw(4) << time->year() << std::setfill('0') << " " <<
        std::setw(2) << time->hour() << std::setfill('0') << ":" <<
        std::setw(2) << time->minute() << std::setfill('0') << ":" <<
        std::setw(2) << time->second() << std::setfill('0');
    
    SAFE_DELETE(time);
    
    return ss.str();
}

//! Split a string by separator char
StringVector SplitString(const std::string& str, char separator);

//! Replaces all occurrences of a substring with another
/*! \param str String to modify
    \param replace_this What substring to replace
    \param replace_with Substring to replace with
 */
std::string ReplaceSubstring(const std::string &str, const std::string &replace_this, const std::string &replace_with);    

//! Replaces certain char in string with another char
/*! \param str String to modify
    \param replace_this What char to replace
    \param replace_with Char to replace with
 */
std::string ReplaceChar(const std::string& str, char replace_this, char replace_with);

//! Replaces all occurrences of a substring with another
/*! \param str String to modify
    \param replace_this What substring to replace
    \param replace_with Substring to replace with
 */
void ReplaceSubstringInplace(std::string &str, const std::string &replace_this, const std::string &replace_with);    

//! Replaces certain char in string with another char
/*! \param str String to modify
    \param replace_this What char to replace
    \param replace_with Char to replace with
 */
void ReplaceCharInplace(std::string& str, char replace_this, char replace_with);    

#endif


