// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ConfigurationManager_h
# error "Never use <ConfigurationManager-templates.h> directly; include <ConfigurationManager.h> instead."
#endif

#include <Poco/Exception.h>
#include <limits>
#include <string>
namespace Foundation
{
   
    template <typename T > T ConfigurationManager::DeclareSetting(const std::string& group, const std::string& key, 
	    					    							      const T& defaultValue, bool save
		    					    						      ) const
     {	
	    T value = defaultValue;
	    if (!configuration_.isNull())
            {
			    // Generate search key 
                std::string groupKey = group + "." + key;
		        try     
			    {   
				    value = boost::lexical_cast<T>(configuration_->getString(groupKey));     
			    }
		        catch ( Poco::NotFoundException& /*ob*/)
			    {
				    // If key does not exist, NotFoundException is thrown. 

                    // Sanity check, is key allready saved into values map?
                    std::map<string_pair_t, std::string>::iterator iter = values_.find(std::make_pair(group, key));
                    if ( iter != values_.end() )
                    {
                        /// @note PENDING What to do if sanity is loss ?
                    }
			    }
            }
         if ( save ) 
            values_[std::make_pair(group, key)] = boost::lexical_cast<std::string>(value);
        
	    return value;

    }

    template <typename T> void ConfigurationManager::SetSetting(const std::string& group, const std::string& key, const T& value)
    {
        std::string groupKey = group + "." + key;
        std::map<string_pair_t, std::string>::iterator iter = values_.find(std::make_pair(group, key));
        if ( iter != values_.end() )  
            iter->second = boost::lexical_cast<std::string>(value);
        else
            values_[std::make_pair(group, key)] = boost::lexical_cast<std::string>(value);
    }

    template <typename T> T ConfigurationManager::GetSetting(const std::string& group, const std::string& key) const 
    {
        std::string groupKey = group + "." + key;
        T value;
        std::map<string_pair_t, std::string>::iterator iter = values_.find(std::make_pair(group, key));
        if ( iter != values_.end() )    
            value = boost::lexical_cast<T>(iter->second);
        else
            value = boost::lexical_cast<T>(std::numeric_limits<double>::quiet_NaN());
    
        return value;
    }

    template <> std::string ConfigurationManager::GetSetting(const std::string& group, const std::string& key) const
    {
        std::string groupKey = group + "." + key;
        std::string value = "";
        std::map<string_pair_t, std::string>::iterator iter = values_.find(std::make_pair(group, key));
        if ( iter != values_.end() )    
            value = boost::lexical_cast<std::string>(iter->second);
        else
            value ="";
    
        return value;

    }


    template <> std::string ConfigurationManager::GetSettingFromFile(const std::string& group, const std::string& key) const
    {
        std::string groupKey = group + "." + key;
        
        std::string value = "";
        
        try
        {
            value =  configuration_->getString(groupKey);
        }
        catch ( Poco::NotFoundException& /*ob*/)
        {}
        
        return value;

    }


    template <typename T> T ConfigurationManager::GetSettingFromFile(const std::string& group, const std::string& key) const
    {
        std::string groupKey = group + "." + key;
        
        T value;
        
        try
        {
            value =  boost::lexical_cast<T>(configuration_->getString(groupKey));
        }
        catch ( Poco::NotFoundException& /*ob*/)
        {
            // Setting was not found, return NAN value
            value = boost::lexical_cast<T>(std::numeric_limits<double>::quiet_NaN());
        }
        return value;
    }

}