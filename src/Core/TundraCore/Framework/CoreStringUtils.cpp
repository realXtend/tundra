// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "CoreStringUtils.h"

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
    return QStringFromWCharArray(str.data(), (int)str.size());
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

bool ParseBool(QString value)
{
    value = value.trimmed().toLower();
    if (value.isEmpty())
        return false;
    if (value == "1")
        return true;
    if (value == "on")
        return true;
    if (value == "true")
        return true;
    return false;
}
