// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConfigurationManager.h"

namespace Foundation
{
    const char *ConfigurationManager::DEFAULT_CONFIG_PATH = "./data/app_config.xml";

    ConfigurationManager::ConfigurationManager(Type type) : type_(CT_CUSTOM)
    {
        // Do not use logger here - not initialized yet
        assert (type == ConfigurationManager::CT_DEFAULT);
        Load(DEFAULT_CONFIG_PATH);

        type_ = CT_DEFAULT;
    }

    ConfigurationManager::ConfigurationManager(const std::string &file) : type_(CT_CUSTOM)
    {
        Load(file);
    }

    ConfigurationManager::~ConfigurationManager()
    {
        ExportSettings(config_file_);
    }

    void ConfigurationManager::Load(const std::string &file)
    {
        // Do not use logger here - not initialized yet

        assert (type_ != CT_DEFAULT);

        config_file_ = file;
        try
        {
            configuration_ = new Poco::Util::XMLConfiguration(file);
        } catch (std::exception)
        {
            // not fatal in debug mode
#ifndef _DEBUG
            std::string what = std::string("Failed to load configuration file: ") + file;
            throw Core::Exception(what.c_str());
#endif
        }
    }

    bool ConfigurationManager::HasKey(const std::string &group, const std::string &key) const
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
    }
}



