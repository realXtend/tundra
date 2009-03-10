// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConfigurationManager.h"

namespace Foundation
{
    const char *ConfigurationManager::DEFAULT_CONFIG_PATH = "./data/app_config.xml";

    ConfigurationManager::ConfigurationManager(Type type) : type_(CT_CUSTOM)
    {
        assert (type == ConfigurationManager::CT_DEFAULT);
        Load(DEFAULT_CONFIG_PATH);

        type_ = CT_DEFAULT;
    }

    ConfigurationManager::ConfigurationManager() : type_(CT_CUSTOM)
    {
    }

    ConfigurationManager::ConfigurationManager(const std::string &file) : type_(CT_CUSTOM)
    {
        Load(file);
    }

    ConfigurationManager::~ConfigurationManager()
    {
#ifdef EXPORT_CONFIGURATION
        ExportSettings(config_file_);
#endif
    }

    void ConfigurationManager::Load(const std::string &file)
    {
        assert (type_ != CT_DEFAULT);

        config_file_ = file;
        try
        {
            configuration_ = new Poco::Util::XMLConfiguration(file);
        } catch (std::exception &e)
        {
            // not fatal

            LOGERROR(e.what());
            LOGERROR("Failed to load application configuration file " + file + ".");
        }
    }

    int ConfigurationManager::DeclareSetting(const std::string &group, const std::string &key, int defaultValue)
    {
        int value = defaultValue;
        if (configuration_.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            value = configuration_->getInt(groupKey, defaultValue);
        }
#ifdef EXPORT_CONFIGURATION
        values_[std::make_pair(group, key)] = boost::lexical_cast<std::string>(value);
#endif
        return value;
    }

    std::string ConfigurationManager::DeclareSetting(const std::string &group, const std::string &key, const std::string &defaultValue)
    {
        std::string value = defaultValue;
        if (configuration_.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            value = configuration_->getString(groupKey, defaultValue);
        }
#ifdef EXPORT_CONFIGURATION
        values_[std::make_pair(group, key)] = value;
#endif
        return value;
    }

    std::string ConfigurationManager::DeclareSetting(const std::string &group, const std::string &key, const char *defaultValue)
    {
        return DeclareSetting(group, key, std::string(defaultValue));
    }

    bool ConfigurationManager::DeclareSetting(const std::string &group, const std::string &key, bool defaultValue)
    {
        bool value = defaultValue;
        if (configuration_.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            value = configuration_->getBool(groupKey, defaultValue);
        }
#ifdef EXPORT_CONFIGURATION
        values_[std::make_pair(group, key)] = boost::lexical_cast<std::string>(value);
#endif
        return value;
    }

    Core::Real ConfigurationManager::DeclareSetting(const std::string &group, const std::string &key, Core::Real defaultValue)
    {
        Core::Real value = defaultValue;
        if (configuration_.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            value = static_cast<Core::Real>(configuration_->getDouble(groupKey, defaultValue));
        }
#ifdef EXPORT_CONFIGURATION
        values_[std::make_pair(group, key)] = boost::lexical_cast<std::string>(value);
#endif
        return value;
    }

    bool ConfigurationManager::HasKey(const std::string &group, const std::string &key)
    {
        if (configuration_.isNull() == false)
        {
            std::string groupKey = group + "." + key;
            return configuration_->hasProperty(groupKey);
        }
        return false;
    }

    void ConfigurationManager::ExportSettings(const std::string &file)
    {
#ifdef EXPORT_CONFIGURATION
        std::fstream file_op(file.c_str(), std::ios::out);

        Poco::XML::XMLWriter writer(file_op, Poco::XML::XMLWriter::CANONICAL | Poco::XML::XMLWriter::PRETTY_PRINT);
	    writer.startDocument();
	    writer.startElement("", "", "config");

        std::string currentGroup;
        ValueMap::const_iterator iter = values_.begin();
        for ( ; iter != values_.end() ; ++iter)
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



