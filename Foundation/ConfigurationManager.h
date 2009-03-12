// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ConfigurationManager_h
#define incl_Foundation_ConfigurationManager_h

namespace Foundation
{
    class Framework;

    //! A configuration manager for immutable name-value pair settings.
    /*! The settings are divided into groups. Groups can be freely
        defined.

        Usage:
            Everytime you define a constant in a class, declare it
            in this manager instead of hardcoding it. For example:

            \code
class foo
{
    foo() : ConfigurationManager.declareSetting("foo", "var", 0)
    const int var;
};
            \endcode
        

        The settings are cached as soon as a configuration file is opened,
        this behaviour comes from PoCo's XmlConfiguration class and can't
        be changed at present time.
    
        These settings are for application constants, they are
        meant to be used by developers and not by the application
        users. There is no way to set these settings, only read them.        

        Uses PoCo's XmlConfiguration and XmlWriter internally.

        Currently does exporting in a really silly way, but only
        because Poco doesn't allow for writing of XmlConfiguration,
        only reading.

        \note When EXPORT_CONFIGURATION is defined, the configuration file is
              written out with default values after this manager is destroyed.

        \note This class makes no assumptions about the type of the stored
              values, it is assumed the programmer always knows it.
    */
    class ConfigurationManager
    {
        friend class Framework;

        //! Special enum for creating default configuration object
        enum Type
        {
            //! Default configuration created by the framework, uses default config file.
            CT_DEFAULT,
            //! Custom configuration, uses custom config file.
            CT_CUSTOM
        };

        //! special constructor for creating default configuration object
        ConfigurationManager(Type type = CT_DEFAULT);

        ConfigurationManager(const ConfigurationManager &other);
        ConfigurationManager &operator =(const ConfigurationManager &other);

    public:
        //! constructor that takes a path to a configuration file. Loads and parses the file.
        ConfigurationManager(const std::string &file);

        //! destructor. 
        /*! 
            \note When EXPORT_CONFIGURATION is defined, exports the settings to the same
                  file from where they were originally loaded from.
        */
        ~ConfigurationManager();

        //! Loads and parses configuration from the specified file.
        void Load(const std::string &file);

        //! Declares a key-value pair setting.
        /*! This is the standard way of retrieving a value from group and key name for the first
            time.

            Postcond: HasKey(group, key)

            For retrieving the value later, use GetSetting((const std::string &group, const std::string &key) const;

            \param group Group the key belongs to
            \param key Name of the key
            \param defaultValue default value for the setting, returned if key was not found
        */
        int DeclareSetting(const std::string &group, const std::string &key, int defaultValue) const;

        //! \see DeclareSetting(const std::string &group, const std::string &name, int defaultValue);
        std::string DeclareSetting(const std::string &group, const std::string &key, const std::string &defaultValue) const;

        //! \see DeclareSetting(const std::string &group, const std::string &name, int defaultValue);
        std::string DeclareSetting(const std::string &group, const std::string &key, const char *defaultValue) const;

        //! \see DeclareSetting(const std::string &group, const std::string &name, int defaultValue);
        bool DeclareSetting(const std::string &group, const std::string &key, bool defaultValue) const;

        //! \see DeclareSetting(const std::string &group, const std::string &name, int defaultValue);
        Core::Real DeclareSetting(const std::string &group, const std::string &key, Core::Real defaultValue) const;

        //! Retrieves a value from key and group.
        /*! This is the standard way of retrieving a value from group and key name, after the
            key has already been declared earlier.

            Precond: HasKey(group, key)

            \note Before using this function, declare the setting with 
                  DeclareSetting(const std::string &group, const std::string &key, int defaultValue = 0) const

            \param group Group the key belongs to
            \param key Name of the key
        */
        int GetInt(const std::string &group, const std::string &key) const
        {
#ifdef _DEBUG
            return boost::lexical_cast<int>(values_.find(std::make_pair(group, key))->second);
#else
            std::string groupKey = group + "." + key;
            return configuration_->getInt(groupKey);
#endif
        }

        //! \see GetInt(const std::string &group, const std::string &name, int defaultValue);
        std::string GetString(const std::string &group, const std::string &key) const
        {
#ifdef _DEBUG
            return values_.find(std::make_pair(group, key))->second;
#else
            std::string groupKey = group + "." + key;
            return configuration_->getString(groupKey);
#endif
        }

        //! \see GetInt(const std::string &group, const std::string &name, int defaultValue);
        bool GetBool(const std::string &group, const std::string &key) const
        {
#ifdef _DEBUG
            return boost::lexical_cast<bool>(values_.find(std::make_pair(group, key))->second);
#else
            std::string groupKey = group + "." + key;
            return configuration_->getBool(groupKey);
#endif
        }

        //! \see GetInt(const std::string &group, const std::string &name, int defaultValue);
        Core::Real GetReal(const std::string &group, const std::string &key) const
        {
#ifdef _DEBUG
            return boost::lexical_cast<Core::Real>(values_.find(std::make_pair(group, key))->second);
#else
            std::string groupKey = group + "." + key;
            return static_cast<Core::Real>(configuration_->getDouble(groupKey));
#endif
        }

        //! Returns true is the specified group contains the specified key, false otherwise.
        /*!
            \param group Name of the group
            \param name Name of the key
        */
        bool HasKey(const std::string &group, const std::string &key) const;
        
        
    private:
        //! Export current settings to specified file
        /*! Does silly trickery when EXPORT_CONFIGURATION in defined to export the settings.

            \param file path to file to export settings to
        */
        void ExportSettings(const std::string &file);

        //! default configuration file path
        static const char *DEFAULT_CONFIG_PATH;

        typedef std::pair<std::string, std::string> string_pair_t;
        typedef std::map<string_pair_t, std::string> ValueMap;

        //! Poco xml configuration reader
        Poco::AutoPtr<Poco::Util::XMLConfiguration> configuration_;
		
        //! Current configuration file
        std::string config_file_;

        //! Type of this config
        Type type_;

#ifdef EXPORT_CONFIGURATION
        //! map of all values, for exporting
        ValueMap values_;
#endif
    };
}

#endif

