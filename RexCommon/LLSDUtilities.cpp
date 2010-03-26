// For conditions of distribution and use, see copyright notice in license.txt
#include "LLSDUtilities.h"

namespace RexTypes
{
    bool GetNextLLSDTag(const std::string& tag, const std::string& data, std::size_t& index)
    {
        for (;;)
        {
            std::string next_tag = GetNextLLSDTag(data, index);
            if (next_tag.empty())
                return false;
            if (next_tag.find(tag) != std::string::npos)
                return true;
        }
    }
    
    std::string GetNextLLSDTag(const std::string& data, std::size_t& index)
    {
        if (index >= data.length())
            return std::string();
        
        std::size_t begin, end;
        
        for (;;)
        {
            begin = data.find('<', index);
            if (begin == std::string::npos)
                return std::string();
            end = data.find('>', begin);
            if (end == std::string::npos)
                return std::string();

            index = end + 1;

            if (end - begin <= 1)
                return std::string();
            
            if (data[begin + 1] != '/')
                break;
        }
        
        return data.substr(begin + 1, end - begin - 1);
    }
    
    std::string GetLLSDTagContent(const std::string& data, std::size_t& index)
    {
        std::size_t begin = index;
        std::size_t end = data.find('<', index);
        if (end == std::string::npos)
            end = data.length();
        index = end;
        
        return data.substr(begin, end - begin);
    }
        
    std::map<std::string, std::string> ParseLLSDMap(const std::string& data)
    {
        // Note: we don't use the QT xml parser for example, because for instance the legacy avatar data contains 
        // &lt; &gt; pairs intended to fool the parser, but QT xml parser does not get fooled
        std::size_t index = 0;
        std::map<std::string, std::string> contents;
        
        if (GetNextLLSDTag("map", data, index))
        {
            for (;;)
            {
                if (!GetNextLLSDTag("key", data, index))
                    break;
                std::string key = GetLLSDTagContent(data, index);
                // Next tag may be any type, but we store the contents as a string
                if (GetNextLLSDTag(data, index).empty())
                    break;
                std::string value = GetLLSDTagContent(data, index);
                contents[key] = value;
            }
        }
        
        return contents;
    }
}

