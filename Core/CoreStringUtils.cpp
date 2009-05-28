#include "CoreStableHeaders.h"

namespace Core
{
    Core::StringVector SplitString(const std::string& str, char separator)
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
}

