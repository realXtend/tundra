// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_StringUtil_h
#define incl_Core_StringUtil_h

#include <iomanip>

// Disable warnings C4702 coming from boost
#ifdef _MSC_VER
#pragma warning ( push )
#pragma warning( disable : 4702 )
#endif

#include <boost/lexical_cast.hpp>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "CoreDefines.h"
#include "CoreTypes.h"

#include <QString>

QString QStringfromWCharArray(const wchar_t *string, int size);
int QStringtoWCharArray(QString qstr, wchar_t *array);
std::wstring QStringToWString(const QString &qstr);
QString WStringToQString(const std::wstring &str);

std::wstring ToWString(const std::string &str);

/// Converts value to a string. May throw boost::bad_lexical_cast.
template <class T>
std::string ToString(const T &val) { return boost::lexical_cast<std::string>(val); }

/// Converts string to a primitive type, such as int or float. May throw boost::bad_lexical_cast.
template <typename T>
T ParseString(const std::string &val) { return boost::lexical_cast<T>(val); }

/// Converts string to a primitive type, such as int or float. Returns default value on boost::bad_lexical_cast
template <typename T>
T ParseString(const std::string &val, T default_value) 
{
    try
    {
        return boost::lexical_cast<T>(val); 
    }
    catch(boost::bad_lexical_cast e)
    {
        return default_value;
    }
}

/// Get the current time as a string.
std::string GetLocalTimeString();

/// Get the current date and time as a string.
std::string GetLocalDateTimeString();

/// Split a string by separator char
StringVector SplitString(const std::string& str, char separator);

/// Replaces all occurrences of a substring with another
/** \param str String to modify
    \param replace_this What substring to replace
    \param replace_with Substring to replace with
 */
std::string ReplaceSubstring(const std::string &str, const std::string &replace_this, const std::string &replace_with);    

/// Replaces certain char in string with another char
/** \param str String to modify
    \param replace_this What char to replace
    \param replace_with Char to replace with
 */
std::string ReplaceChar(const std::string& str, char replace_this, char replace_with);

/// Replaces all occurrences of a substring with another
/** \param str String to modify
    \param replace_this What substring to replace
    \param replace_with Substring to replace with
 */
void ReplaceSubstringInplace(std::string &str, const std::string &replace_this, const std::string &replace_with);    

/// Replaces certain char in string with another char
/** \param str String to modify
    \param replace_this What char to replace
    \param replace_with Char to replace with
 */
void ReplaceCharInplace(std::string& str, char replace_this, char replace_with);    

/// Convert vector to string
std::string BufferToString(const std::vector<s8>& buffer);

/// Convert string to vector
std::vector<s8> StringToBuffer(const std::string& str);

/// Calculate SDBM hash for a string
uint GetHash(const std::string& str);

/// Calculate SDBM hash for a string
uint GetHash(const QString& str);

/// Parses boolean value from string.
/** The value can either true, false (both case-insensitive), 1 or 0.
    @param value String to be inspected.
*/
bool ParseBool(const std::string &value);

/// This is an overloaded function.
/** @param value String to be inspected.
*/
bool ParseBool(const QString &value);

#endif
