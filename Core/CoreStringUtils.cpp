#include "CoreStableHeaders.h"
#include "CoreStringUtils.h"

StringVector SplitString(const std::string& str, char separator)
{
    std::vector<std::string> vec;
    unsigned pos = 0;

    while (pos < str.length())
    {
        unsigned start = pos;
        
        while (start < str.length())
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

        while (end < str.length())
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
    while (index != std::string::npos)
    {
        str.replace(index, replace_this.length(), replace_with);
        index = str.find(replace_this, 0);
    }
}


void ReplaceCharInplace(std::string& str, char replace_this, char replace_with)
{
    for (uint i = 0; i < str.length(); ++i)
        if (str[i] == replace_this) str[i] = replace_with;
}    


