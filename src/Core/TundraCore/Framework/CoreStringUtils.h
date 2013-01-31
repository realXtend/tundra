// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"

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

/// Converts string to a float, returns defaultValue if conversion fails.
inline float ParseFloat(const QString &val, float defaultValue) { bool ok; float ret = val.toFloat(&ok); return ok ? ret : defaultValue; }
inline float ParseFloat(const std::string &val, float defaultValue) { return ParseFloat(QString::fromStdString(val), defaultValue); } /**< @overload */

/// Converts string to an integer, returns defaultValue if conversion fails.
inline int ParseInt(const QString &val, int defaultValue) { bool ok; int ret = val.toInt(&ok); return ok ? ret : defaultValue; }
inline float ParseInt(const std::string &val, int defaultValue) { return ParseInt(QString::fromStdString(val), defaultValue); } /**< @overload */

/// Converts string to an unsigned integer, returns defaultValue if conversion fails.
inline uint ParseUInt(const QString &val, uint defaultValue) { bool ok; uint ret = val.toUInt(&ok); return ok ? ret : defaultValue; }
inline float ParseUInt(const std::string &val, uint defaultValue) { return ParseInt(QString::fromStdString(val), defaultValue); } /**< @overload */

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
inline uint ComputeHash(const QString &str) { return ComputeHash(str.toStdString()); } /**< @overload */
inline uint GetHash(const std::string &str) { return ComputeHash(str); } /**< @deprecated Use ComputeHash */
inline uint GetHash(const QString &str) { return ComputeHash(str); } /**< @deprecated Use ComputeHash */

/// Parses boolean value from string, case-insensitive.
/** Accepted variations are on/off, true/false & 0/1* /
    @param value String to be inspected. */
bool TUNDRACORE_API ParseBool(QString value);
inline bool ParseBool(const std::string &value) { return ParseBool(QString::fromStdString(value)); } /**< @overload */

/// Converts boolean to "true" or "false".
inline QString BoolToString(bool value) { return value ? "true" : "false"; }
