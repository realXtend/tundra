// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ConfigurationManager_h
# error "Never use <ConfigurationManager-templates.h> directly; include <ConfigurationManager.h> instead."
#endif

#include <limits>
#include <string>

#include "boost/lexical_cast.hpp" 

namespace Foundation
{
    template <typename T > T ConfigurationManager::DeclareSetting(const std::string& group, const std::string& key, 
                                                                  const T& defaultValue) const
     {    
        if (HasKey(group, key))
            return GetSetting<T>(group,key);
        else
            values_[std::make_pair(group, key)] = boost::lexical_cast<std::string>(defaultValue);

        return defaultValue;
    }

    template <typename T> void ConfigurationManager::SetSetting(const std::string& group, const std::string& key, const T& value)
    {
        std::map<string_pair_t, std::string>::iterator iter = values_.find(std::make_pair(group, key));
        if (iter != values_.end())
            iter->second = boost::lexical_cast<std::string>(value);
        else
            values_[std::make_pair(group, key)] = boost::lexical_cast<std::string>(value);
    }

    template <> inline std::string ConfigurationManager::GetSetting<std::string>(const std::string& group, const std::string& key) const
    {
        std::string value = "";
        std::map<string_pair_t, std::string>::iterator iter = values_.find(std::make_pair(group, key));
        if (iter != values_.end())
            value = boost::lexical_cast<std::string>(iter->second);
        else
            value ="";

        return value;
    }

    template <typename T> inline T ConfigurationManager::GetSetting(const std::string& group, const std::string& key) const
    {
        T value;
        std::map<string_pair_t, std::string>::iterator iter = values_.find(std::make_pair(group, key));
        if (iter != values_.end())
            value = boost::lexical_cast<T>(iter->second);
        else
            value = boost::lexical_cast<T>(std::numeric_limits<double>::quiet_NaN());

        return value;
    }
}
