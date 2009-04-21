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
            Everytime you define a constant (or property) in a class, declare it
            in this manager instead of hardcoding it. For example:

            \code
				class foo
				{
				  foo() : ConfigurationManager.declareSetting("foo", "var", 0, true)
				 const int var;
				};
            \endcode
        

        The settings are cached as soon as a configuration file is opened,
        this behaviour comes from PoCo's XmlConfiguration class and can't
        be changed at present time.
    
        These settings are application variables, which can be used to define application runtime and static 
        properties. Depending if setting is set to saved, it will be saved into configuration xml file and it
        is changeble/readable from there. 
        
        Uses PoCo's XmlConfiguration and XmlWriter internally.

        Currently does exporting in a really silly way, but only
        because Poco doesn't allow for writing of XmlConfiguration,
        only reading.

        \note Settings are saved when ConfigurationManager is destroyed, or if class user calls 
        @p ExportSettings() method.

        \note This class makes no assumptions about the type of the stored
              values, it is assumed the programmer always knows it.
    */
    class ConfigurationManager
    {
      
        //! Special enum for creating default configuration object
        enum Type
        {
            //! Default configuration created by the framework, uses default config file.
            CT_DEFAULT,
            //! Custom configuration, uses custom config file.
            CT_CUSTOM
        };

       
        ConfigurationManager(const ConfigurationManager &other);
        ConfigurationManager &operator =(const ConfigurationManager &other);

    public:
        //! Constructor that takes a path to a configuration file. Loads and parses the file.
        ConfigurationManager(Framework* framework, const std::string &file = std::string(DEFAULT_CONFIG_PATH));
		
        //! Destructor. 
        /*! 
           @note when destructor is called, all settings which are saved into value map will be write to the given xml-file. 
        */
        ~ConfigurationManager();

        //! Loads and parses configuration from the specified file.
        void Load(const std::string &file);

        //! Declares a key-value pair setting
        /*! This is the standard way of retrieving a value from group and key name for the first
            time, and also setting a default value if the setting doesn't exist yet.

            Postcond: HasKey(group, key)
           
           @code
                // Retrieve key 
                bool myValue = framework_->GetDefaultConfigPtr()->GetSettingFromFile<bool>(std::string("mygroup"), std::string("mykey"));
            @endcode
           
            \param group Group the key belongs to
            \param key Name of the key
            \param defaultValue default value for the setting, returned if key was not found
            \param save Save defines if setting is saved into xml file or not, default value is true
        */
      
		template <typename T > T DeclareSetting(const std::string& group, const std::string& key, const T& defaultValue, bool save = true) const;

        /**
         * Sets a given value to setting value. If class user wants to change current setting (on runtime) value he/she must use this method to do it.
         *  
         * @param group Group the key belongs to
         * @param key Name of the key
         * @param value Value which will be set to. 
         */
        template <typename T> void SetSetting(const std::string& group, const std::string& key, const T& value);

        /**
         * Retrieves a value from key and group from in-memory. 
         * If value is not found empty string or NAN is return. 
         * 
         * @param group Group the key belongs to
         * @param key Name of the key
         */
        template <typename T> T GetSetting(const std::string& group, const std::string& key) const;
        template <> std::string GetSetting(const std::string& group, const std::string& key) const;

        //! Retrieves a value from key and group.
        /*! This is the standard way of retrieving a value from group and key name, after the
          
            \note if setting is not declared before and there is nothing saved in xml file, function will return NAN value. 
          
            \param group Group the key belongs to
            \param key Name of the key
        */

        template <typename T> T GetSettingFromFile(const std::string& group, const std::string& key) const;
        template <> std::string GetSettingFromFile(const std::string& group, const std::string& key) const;
       
        
        //! Returns true is the specified group contains the specified key, false otherwise.
        /*!
            \param group Name of the group
            \param key Name of the key
        */
        bool HasKey(const std::string &group, const std::string &key) const;
            
        //! Export current settings to specified file
        /*! Does silly trickery to export the settings.

            \param file path to file to export settings to
        */
        void ExportSettings(const std::string &file = std::string(DEFAULT_CONFIG_PATH));

        
    private:
      

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

        //! map of all values, for exporting
        mutable ValueMap values_;

		Framework *framework_;
    };

}
#include "ConfigurationManager-templates.h"
#endif

