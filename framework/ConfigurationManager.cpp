// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"



#include "ConfigurationManager.h"

namespace Foundation
{
    const char *ConfigurationManager::DEFAULT_CONFIG_PATH = "./data/app_config.xml";

    ConfigurationManager::ConfigurationManager()
    {
        load(DEFAULT_CONFIG_PATH);
    }

    ConfigurationManager::ConfigurationManager(const std::string &file)
    {
        load(file);
    }

    ConfigurationManager::~ConfigurationManager()
    {
#ifdef EXPORT_CONFIGURATION
        exportSettings(mConfigFile);
#endif
    }

    void ConfigurationManager::load(const std::string &file)
    {
        mConfigFile = file;
        try
        {
            mConfiguration = new Poco::Util::XMLConfiguration(file);
        } catch (std::exception &e)
        {
            // not fatal

            LOGERROR(e.what());
            LOGERROR("Failed to load application configuration file " + file + ".");
        }
    }

    int ConfigurationManager::declareSetting(const std::string &group, const std::string &key, int defaultValue)
    {
        int value = defaultValue;
        if (mConfiguration.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            value = mConfiguration->getInt(groupKey, defaultValue);
        }
#ifdef EXPORT_CONFIGURATION
        mValues[std::make_pair(group, key)] = boost::lexical_cast<std::string>(value);
#endif
        return value;
    }

    std::string ConfigurationManager::declareSetting(const std::string &group, const std::string &key, const std::string &defaultValue)
    {
        std::string value = defaultValue;
        if (mConfiguration.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            value = mConfiguration->getString(groupKey, defaultValue);
        }
#ifdef EXPORT_CONFIGURATION
        mValues[std::make_pair(group, key)] = value;
#endif
        return value;
    }

    std::string ConfigurationManager::declareSetting(const std::string &group, const std::string &key, const char *defaultValue)
    {
        return declareSetting(group, key, std::string(defaultValue));
    }

    bool ConfigurationManager::declareSetting(const std::string &group, const std::string &key, bool defaultValue)
    {
        bool value = defaultValue;
        if (mConfiguration.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            value = mConfiguration->getBool(groupKey, defaultValue);
        }
#ifdef EXPORT_CONFIGURATION
        mValues[std::make_pair(group, key)] = boost::lexical_cast<std::string>(value);
#endif
        return value;
    }

    Core::Real ConfigurationManager::declareSetting(const std::string &group, const std::string &key, Core::Real defaultValue)
    {
        Core::Real value = defaultValue;
        if (mConfiguration.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            value = static_cast<Core::Real>(mConfiguration->getDouble(groupKey, defaultValue));
        }
#ifdef EXPORT_CONFIGURATION
        mValues[std::make_pair(group, key)] = boost::lexical_cast<std::string>(value);
#endif
        return value;
    }

    bool ConfigurationManager::hasKey(const std::string &group, const std::string &key)
    {
        if (mConfiguration.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            return mConfiguration->hasProperty(groupKey);
        }
        return false;
    }

    void ConfigurationManager::exportSettings(const std::string &file)
    {
#ifdef EXPORT_CONFIGURATION
        std::fstream file_op(file.c_str(), std::ios::out);

        Poco::XML::XMLWriter writer(file_op, Poco::XML::XMLWriter::CANONICAL | Poco::XML::XMLWriter::PRETTY_PRINT);
	    writer.startDocument();
	    writer.startElement("", "", "config");

        std::string currentGroup;
        ValueMap::const_iterator iter = mValues.begin();
        for ( ; iter != mValues.end() ; ++iter)
        {
            if ( currentGroup.empty() == false && 
                 currentGroup != iter->first.first )
            {
                writer.endElement("", "", currentGroup);
            }
            if ( currentGroup.empty() == true || 
                 currentGroup != iter->first.first )
            {
                writer.startElement("", "", iter->first.first);
                currentGroup = iter->first.first;
            }
            {
                writer.startElement("", "", iter->first.second);
	            writer.rawCharacters(iter->second);
                writer.endElement("", "", iter->first.second);
            }
        }
        if (currentGroup.empty() == false)
        {
            writer.endElement("", "", currentGroup);
        }
        
	    writer.endElement("", "", "config");
	    writer.endDocument();
#endif
    }
}



