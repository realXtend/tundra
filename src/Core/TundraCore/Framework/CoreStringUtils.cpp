// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "CoreStringUtils.h"

#include <QTextStream>
#include <QByteArray>
#include <QRegExpValidator>

#include <kNet/DataSerializer.h>
#include <kNet/DataDeserializer.h>

QString QStringFromWCharArray(const wchar_t *string, int size)
{
    QString qstr;
    if (sizeof(wchar_t) == sizeof(QChar))
        return qstr.fromUtf16((const ushort *)string, size);
    else
        return qstr.fromUcs4((uint *)string, size);
}

int QStringToWCharArray(const QString &qstr, wchar_t *array)
{
    if (sizeof(wchar_t) == sizeof(QChar))
    {
        memcpy(array, qstr.utf16(), sizeof(wchar_t)*qstr.length());
        return qstr.length();
    }
    else
    {
        wchar_t *a = array;
        const unsigned short *uc = qstr.utf16();
        for (int i = 0; i < qstr.length(); ++i)
        {
            uint u = uc[i];
            if (QChar(u).isHighSurrogate() && i + 1 < qstr.length())
            {
                ushort low = uc[i+1];
                if (QChar(low).isLowSurrogate())
                {
                    u = QChar::surrogateToUcs4(u, low);
                    ++i;
                }
            }
            *a = wchar_t(u);
            ++a;
        }
        return a - array;
    }
}

std::wstring QStringToWString(const QString &qstr)
{
    if (qstr.length() == 0)
        return L"";

    std::wstring str;
    str.resize(qstr.length());

    str.resize(QStringToWCharArray(qstr, &(*str.begin())));
    return str;
}

QString WStringToQString(const std::wstring &str)
{
    if (str.length() == 0)
        return "";
    return QStringFromWCharArray(str.data(), str.size());
}

std::wstring ToWString(const std::string &str)
{
    std::wstring w_str(str.length(), L' ');
    std::copy(str.begin(), str.end(), w_str.begin());
    return w_str;
}

std::string BufferToString(const std::vector<s8>& buffer)
{
    if (!buffer.empty())
        return std::string((const char*)&buffer[0], buffer.size());
    else
        return std::string();
}

std::vector<s8> StringToBuffer(const std::string& str)
{
    std::vector<s8> ret;
    ret.resize(str.size());
    if (str.size())
        memcpy(&ret[0], &str[0], str.size());
    return ret;
}

QByteArray RemoveLines(const QByteArray &data, QStringList linePrefixes, uint *removedLineCount)
{
    if (removedLineCount != 0)
        *removedLineCount = 0;
        
    QByteArray result;
    QTextStream out(&result, QIODevice::WriteOnly);
    QTextStream in(data, QIODevice::ReadOnly);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        bool found = false;
        foreach(const QString &commentPrefix, linePrefixes)
        {
            found = line.simplified().startsWith(commentPrefix, Qt::CaseSensitive);
            if (found)
                break;
        }
        if (!found)
        {
            // readLine() removes end-of-line characters, preserve them.
            out << line << endl;
        }
        else if (removedLineCount != 0)
            *removedLineCount += 1;
    }
    return result;
}

StringVector SplitString(const std::string& str, char separator)
{
    std::vector<std::string> vec;
    unsigned pos = 0;

    while(pos < str.length())
    {
        unsigned start = pos;
        
        while(start < str.length())
        {
            if (str[start] == separator)
                break;
            
            start++;
        }

        if (start == str.length())
        {
            vec.push_back(str.substr(pos));
            break;
        }
        
        unsigned end = start;

        while(end < str.length())
        {
            if (str[end] != separator)
                break;

            end++;
        }

        vec.push_back(str.substr(pos, start - pos));
        pos = end;
    }

    return vec;
}

std::string ReplaceSubstring(const std::string &str, const std::string &replace_this, const std::string &replace_with)
{
    std::string ret = str;
    ReplaceSubstringInplace(ret, replace_this, replace_with);
    return ret;
}

std::string ReplaceChar(const std::string& str, char replace_this, char replace_with)
{
    std::string ret = str;
    ReplaceCharInplace(ret, replace_this, replace_with);
    return ret;
}

void ReplaceSubstringInplace(std::string &str, const std::string &replace_this, const std::string &replace_with)
{
    std::size_t index = str.find(replace_this, 0);
    while(index != std::string::npos)
    {
        str.replace(index, replace_this.length(), replace_with);
        index = str.find(replace_this, 0);
    }
}

void ReplaceCharInplace(std::string& str, char replace_this, char replace_with)
{
    for(uint i = 0; i < str.length(); ++i)
        if (str[i] == replace_this) str[i] = replace_with;
}

uint ComputeHash(const std::string& str)
{
    uint ret = 0;
    
    if (!str.length())
        return ret;
    
    const char* cstr = str.c_str();
    
    while(*cstr)
    {
        // Note: calculate case-insensitive hash
        char c = *cstr;
        ret = tolower(c) + (ret << 6) + (ret << 16) - ret;
        ++cstr;
    }
    
    return ret;
}

bool ParseBool(QString value, bool valueIfEmpty)
{
    value = value.trimmed().toLower();
    if (value.isEmpty())
        return valueIfEmpty;
    if (value == "1")
        return true;
    if (value == "on")
        return true;
    if (value == "true")
        return true;
    return false;
}

QString ReadUtf8String(kNet::DataDeserializer &dd)
{
    QByteArray utf8Bytes;
    utf8Bytes.resize(dd.Read<u16>());
    if (!utf8Bytes.isEmpty())
    {
        dd.ReadArray<u8>((u8*)utf8Bytes.data(), utf8Bytes.size());
        return QString::fromUtf8(utf8Bytes.data(), utf8Bytes.size());
    }
    else
        return QString();
}

void WriteUtf8String(kNet::DataSerializer& ds, const QString& str)
{
    const QByteArray utf8Bytes = str.toUtf8();
    ds.Add<u16>(utf8Bytes.size());
    if (!utf8Bytes.isEmpty())
        ds.AddArray<u8>((const u8*)utf8Bytes.data(), utf8Bytes.size());
}

QString CamelCase(const QString &str, bool lowerCase)
{
    QString out;
    if (str.contains(" "))
    {
        foreach(QString part, str.split(" ", QString::SkipEmptyParts))
        {
            part[0] = part.at(0).toUpper();
            out += part;
        }
    }
    else
    {
        out = str;
        if (!lowerCase)
            out[0] = out.at(0).toUpper();
    }
    if (lowerCase && out.length() > 0)
        out[0] = out.at(0).toLower();
    return out;
}

bool IsAlphanumeric(QString &str, int &invalidCharPosition)
{
    static QRegExpValidator alphaNumericValidator(QRegExp("[A-Za-z0-9]"));
    return (alphaNumericValidator.validate(str, invalidCharPosition) == QValidator::Acceptable);
}
