// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ConfigurationManager_h
#define incl_Foundation_ConfigurationManager_h

//#include "StableHeaders.h"
#include "boost/filesystem.hpp" 
#include "Poco/Util/XMLConfiguration.h"

namespace Foundation
{
    class Framework;

    //! A configuration manager for immutable name-value pair settings.
    /*! The settings are divided into groups. Groups can be freely
        defined.

        Usage:
            Everytime you define a property which is used as a setting variable in a class (or anywhere), declare it
            in this manager instead of hardcoding it. Example:

            @code
            foo::foo(FrameWork* framework) 
            {
                int myValue = 42;
                myValue = framework->GetConfigManager()->DeclareSetting(std::string("myGroup"), std::string("myKey"), myValue));

                //Another way 

                if ( framework->GetConfigManager()->HasKey(std::string("myGroup"), std::string("myKey"))
                    myValue = framework->GetConfigManager()->GetSetting<int>(std::string("myGroup"), std::string("myKey"));
                else
                    framework->GetConfigManager()->SetSetting(std::string("myGroup"), std::string("myKey"), myValue);


            }
            @endcode
        

        The settings are cached as soon as a configuration files are readed (this is done by default in manager constructor). New settings from arbitary 
        location can be loaded by using @p Load() method. 
       
        These settings are application variables, which can be used to define application runtime and static 
        properties.
        
        Uses PoCo's XmlConfiguration internally.


        @note Settings are saved when ConfigurationManager is destroyed, or if class user calls 
        @p Export() method.

        @note This class makes no assumptions about the type of the stored
              values, it is assumed the programmer always knows it.

        @ingroup Foundation_group
    */

    class ConfigurationManager
    {
    public:
        
        /**
         * Default constructor. 
         * 
         * Note: does not load configuration files by default.
         *
         * @param framework is pointer to current framework (currently not used). 
         * @param path is value which represents path to location where configuration xml files are found. 
         * @note @p path is platform independ (means that it can be given with M$ -style or Unix-style separator)
         */
        
        ConfigurationManager(Framework* framework = 0, const std::string& path = std::string(DEFAULT_CONFIG_PATH));
		
        /**
         * Destructor. 
         * @note when destructor is called, all settings which are saved into runtime value map will be write into corresponding configuration xml files. Files can 
         * be found from location which is defined in path variable. Default path is viewer/data/configuration/. 
         */
        ~ConfigurationManager();
    
        /**
         * Sets new path to be used for both loading and saving.
         * @param path New path for configuration xml files
         */
        void SetPath(const std::string& path);

        /**
         *  Loads xml files from given path and sets found group-key value pairs into configuration manager memory for futher use. If given path is a file
         *  file data will be loaded into runtime memory map, file encoding must match current name encoding see @p SetFileNameEncoding() 
         *  
         *  @note Load() will only read those files which name encoding match to defined file encoding. See @p SetFileEncoding().
         *  @note Load() will always replace existing group-key value pairs, with new value which are found from xml files. 
         *  
         *  @param path is path to folder where configuration files are. If blank, uses one stored in constructor or SetPath().
         *  @throw CoreException if problem arises. 
         */

        void Load(const std::string& path = std::string());

        /**
         * Exports settings into xml data files. XML-files are created in given path. If @p group is other then empty string, 
         * only that group values are exported (into corresponding xml-file). See below how xml files are generated.
         * 
         * Xml configuration files are created so that their name starts with given encoding and right after that second part of their name is
         * same as group name. Example for group "AssetManager" corresponding xml file is RexAssetManager.xml where "Rex" is file encoding variable and 
         * can be changed using @p SetFileNameEncoding().  
         *
         * @note if given path points to file and group is empty string @p Export() function does not export anything. If given path points to file and group 
         * is something else then empty string only that group data will be stored to given file (now file can be arbitary). 
         * @note if given path is a folder and group is diffrent then empty string that group will be only which is exported to given folder.
         *
         * @code
         * std::string myFolderPath="/path/to/folder" // can be unix style or m$-style. 
         * // Exports now only keys which are defined in myGroup.
         * config_manager_->Export(myFolderPath, std::string("myGroup")); 
         * @endcode
         * 
         * @param path where xml files will be stored (folder). If empty, stored to the path set with SetPath().
         * @param group is group which will be exported. If empty string is given method will export all setting from value map to given path. 
         */
        void Export(const std::string& path = std::string(), const std::string& group = std::string());
        
        std::string GetPath() const { return path_.string(); }

        /**
         * Sets a file name encoding. Default encoding is "Rex", so created configuration files starts with letters "Rex". This setting also
         * affects of how @p Load and @p Export searches configuration files from given file path (default case).   
         * @param encoding is encoding which defines what files are valid xml configuration files. 
         *
         **/
        void SetFileNameEncoding(const std::string& encoding) { file_name_encoding_ = encoding; }
        std::string GetFileNameEncoding() const { return file_name_encoding_; }

        /**
         * Removes declared setting from runtime map. 
         * 
         * @param group is the name of the Group. 
         * @param key is the name of the Key. 
         * @note Method does not remove setting from a file. 
         */
        void Remove(const std::string& group, const std::string key);

        /**
         * Declares setting. If searched setting is found from xml file it will be returned 
         * else default value will be set into runtime value map, and stored afterwords in xml-file. 
         * @code
         * 
         *  int myVariable = 42;
         *  // I want to use this again in future or changed directly from xml configuration file (or i want to read newer value from configuration file) 
         *  myVariable = config_manager_.DeclareSetting(std::string("myGroup"), std::string("myValueKey"), myVariable);
         * 
         * @endcode
         * @return given default value if group and key are not found from runtime map. Else returns value which is saved into runtime map. 
         * @param group is the name of Group. 
         * @param key is the name of Key.
         */
        template <typename T > T DeclareSetting(const std::string& group, const std::string& key, const T& defaultValue) const;      

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
         * @code
         *  // I want to get current setting from runtime-map. 
         *  bool all_knowing_truth = config_manager_.GetSetting<bool>(std::string("myGroup"), std::string("myKey"));
         * 
         * @endcode
         * @param group Group the key belongs to
         * @param key Name of the key
         * @throw In very special cases can throw boost::bad_lexical_cast. 
         */

        template <typename T> T GetSetting(const std::string& group, const std::string& key) const;

         /**
          * Returns true is the specified group contains the specified key, false otherwise.
          * @code
          *  // I want to assure that group and key value exist. 
          *  if ( config_manager_.HasKey(std::string("myGroup"), std::string("myKey") )
          *    {
          *      int value = config_manager_.GetSetting<int>(std::string("myGroup"), std::string("myKey"));
          *      // Do something
          *    }
          *   else
          *     // Do something example config_manager_.SetSettting(..);
          * @endcode
          * 
          * @param group Name of the group
          * @param key Name of the key
          *
          */

        bool HasKey(const std::string& group, const std::string& key) const; 

    private:
            
    
        /**
         * Finds all xml files from given directory. Does not search xml files from leaf directories. 
         * @param path is path to directory where xml files are are found. 
         * @return list of file paths. 
         *
         */
        
        std::list<std::string> GetFiles(const boost::filesystem::path& path) const;

        void AddValues(Poco::Util::XMLConfiguration* pConfiguration, const std::string& group);

        // Default configuration file path
        static const char* DEFAULT_CONFIG_PATH;
        
        typedef std::pair<std::string, std::string> string_pair_t;
        typedef std::map<string_pair_t, std::string> ValueMap;

         // map of all values, for exporting
        mutable ValueMap values_;

        boost::filesystem::path path_;
        Framework *framework_;
        std::string file_name_encoding_;


    };

} // namespace
#include "ConfigurationManager-templates.h"
#endif

