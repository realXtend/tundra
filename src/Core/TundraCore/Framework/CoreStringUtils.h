// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"

// Disable warnings C4702 coming from boost
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

/// @cond PRIVATE
class TUNDRACORE_API QStringLessThanNoCase
{
public:
    bool operator()(const QString &a, const QString &b) const
    {
        return QString::compare(a, b, Qt::CaseInsensitive) < 0;
    }
};
/// @endcond

QString TUNDRACORE_API QStringfromWCharArray(const wchar_t *string, int size);

/// Warning: Does not null-terminate the output. Prefer to use QStringToWString instead!
int TUNDRACORE_API QStringtoWCharArray(QString qstr, wchar_t *array);

std::wstring TUNDRACORE_API QStringToWString(const QString &qstr);
QString TUNDRACORE_API WStringToQString(const std::wstring &str);

std::wstring TUNDRACORE_API ToWString(const std::string &str);
std::wstring TUNDRACORE_API QStringToWString(const QString &qstr);
QString TUNDRACORE_API WStringToQString(const std::wstring &str);

/// Converts value to a string. May throw boost::bad_lexical_cast.
template <class T>
std::string ToString(const T &val) { return boost::lexical_cast<std::string>(val); }

/// Converts string to a primitive type, such as int or float. Returns default value on boost::bad_lexical_cast
template <typename T>
T ParseString(const std::string &val, T defaultValue)
{
    try
    {
        return boost::lexical_cast<T>(val);
    }
    catch(boost::bad_lexical_cast e)
    {
        return defaultValue;
    }
}

/// Split a string by separator char
StringVector TUNDRACORE_API SplitString(const std::string& str, char separator);

/// Replaces all occurrences of a substring with another
/** @param str String to modify
    @param replaceThis What substring to replace
    @param replaceWith Substring to replace with */
std::string TUNDRACORE_API ReplaceSubstring(const std::string &str, const std::string &replaceThis, const std::string &replaceWith);

/// Replaces certain char in string with another char
/** @param str String to modify
    @param replaceThis What char to replace
    @param replaceWith Char to replace with */
std::string TUNDRACORE_API ReplaceChar(const std::string& str, char replaceThis, char replaceWith);

/// Replaces all occurrences of a substring with another
/** @param str String to modify
    @param replaceThis What substring to replace
    @param replaceWith Substring to replace with */
void TUNDRACORE_API ReplaceSubstringInplace(std::string &str, const std::string &replaceThis, const std::string &replaceWith);

/// Replaces certain char in string with another char
/** @param str String to modify
    @param replaceThis What char to replace
    @param replaceWith Char to replace with */
void TUNDRACORE_API ReplaceCharInplace(std::string& str, char replaceThis, char replaceWith);

/// Convert vector to string
std::string TUNDRACORE_API BufferToString(const std::vector<s8>& buffer);

/// Convert string to vector
std::vector<s8> TUNDRACORE_API StringToBuffer(const std::string& str);

/// Calculate SDBM hash for a string
uint TUNDRACORE_API ComputeHash(const std::string &str);
inline uint ComputeHash(const QString &str) { return ComputeHash(str.toStdString()); } ///< @overload
inline uint GetHash(const std::string &str) { return ComputeHash(str); } ///< @deprecated Use ComputeHash
inline uint GetHash(const QString &str) { return ComputeHash(str); } ///< @deprecated Use ComputeHash

/// Parses boolean value from string, case-insensitive.
/** Accepted variations are on/off, true/false & 0/1* /
    @param value String to be inspected. */
bool TUNDRACORE_API ParseBool(QString value);
inline bool ParseBool(const std::string &value) { return ParseBool(QString::fromStdString(value)); } ///< @overload

/// Converts boolean to "true" or "false".
inline QString BoolToString(bool value) { return value ? "true" : "false"; }
