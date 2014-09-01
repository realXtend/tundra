// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"

namespace kNet { class DataSerializer; class DataDeserializer; }

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

QString TUNDRACORE_API QStringFromWCharArray(const wchar_t *string, int size);

/// Warning: Does not null-terminate the output. Prefer to use QStringToWString instead!
int TUNDRACORE_API QStringToWCharArray(const QString &qstr, wchar_t *array);

std::wstring TUNDRACORE_API QStringToWString(const QString &qstr);
QString TUNDRACORE_API WStringToQString(const std::wstring &str);

std::wstring TUNDRACORE_API ToWString(const std::string &str);
std::wstring TUNDRACORE_API QStringToWString(const QString &qstr);
QString TUNDRACORE_API WStringToQString(const std::wstring &str);
std::string TUNDRACORE_API WStringToString(const std::wstring &str);

/// Converts string to a float, returns defaultValue if conversion fails.
inline float ParseFloat(const QString &val, float defaultValue) { bool ok; float ret = val.toFloat(&ok); return ok ? ret : defaultValue; }
inline float ParseFloat(const std::string &val, float defaultValue) { return ParseFloat(QString::fromStdString(val), defaultValue); } /**< @overload */

/// Converts string to an integer, returns defaultValue if conversion fails.
inline int ParseInt(const QString &val, int defaultValue) { bool ok; int ret = val.toInt(&ok); return ok ? ret : defaultValue; }
inline int ParseInt(const std::string &val, int defaultValue) { return ParseInt(QString::fromStdString(val), defaultValue); } /**< @overload */

/// Converts string to an unsigned integer, returns defaultValue if conversion fails.
inline uint ParseUInt(const QString &val, uint defaultValue) { bool ok; uint ret = val.toUInt(&ok); return ok ? ret : defaultValue; }
inline uint ParseUInt(const std::string &val, uint defaultValue) { return ParseUInt(QString::fromStdString(val), defaultValue); } /**< @overload */

/// Removes requested lines from data.
/** Parses data line by line, ignoring starting whitespace and matching linePrefixes. Matched lines will be removed.
    String comparisons are done with Qt::CaseSensitive. Useful for removing comment lines from data by passing "//" and/or "#" as linePrefixes.
    @param Data string.
    @param Line prefixes. If a line starts with one of these (after starting whitespace) it will be removed.
    @param Removed line count will be reported here if ptr is not null.
    @return Result data. */
QByteArray TUNDRACORE_API RemoveLines(const QByteArray &data, const QStringList &linePrefixes, uint *removedLineCount = 0);

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
    @param value String to be inspected.
    @param valueIfEmpty Value that is returned if the string is empty. */
bool TUNDRACORE_API ParseBool(QString value, bool valueIfEmpty = false);
inline bool ParseBool(const std::string &value, bool valueIfEmpty = false) { return ParseBool(QString::fromStdString(value), valueIfEmpty); } /**< @overload */

/// Converts boolean to "true" or "false".
inline QString BoolToString(bool value) { return value ? "true" : "false"; }

/// Reads an UTF-8 encoded QString from a data stream
QString TUNDRACORE_API ReadUtf8String(kNet::DataDeserializer &dd);

/// Writes QString to a data stream using UTF-8 encoding.
void TUNDRACORE_API WriteUtf8String(kNet::DataSerializer &ds, const QString &str);

/** Camel-case input string.
    Examples: "My Cool Thing"  -> "myCoolThing"
              "THis aTtribute" -> "tHisATtribute"
    @param str String to convert to camel-case.
    @param lowerCase True if the first character should be lower cased or not. */
QString TUNDRACORE_API CamelCase(const QString &str, bool lowerCase = true);

/// Returns if the string only contains alphanumeric characters with regexp "[A-Za-z0-9]".
bool TUNDRACORE_API IsAlphanumeric(QString &str, int &invalidCharPosition);
