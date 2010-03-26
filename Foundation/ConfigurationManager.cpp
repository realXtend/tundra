// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConfigurationManager.h"
#include "CoreException.h"


namespace Foundation
{
    const char* ConfigurationManager::DEFAULT_CONFIG_PATH = "./data/configuration/";

    ConfigurationManager::ConfigurationManager(Framework* framework, const std::string& path) : path_(boost::filesystem::path(path)), 
        framework_(framework), file_name_encoding_(std::string("Rex"))
    {
    }

    ConfigurationManager::~ConfigurationManager()
    {
	    Export();
        
        // Does not own.
	    framework_ = 0;
    }

    void ConfigurationManager::SetPath(const std::string& path)
    {
        path_ = boost::filesystem::path(path);
    }
    
    void ConfigurationManager::Load(const std::string& path)
    {
        namespace fs = boost::filesystem;

        fs::path filePath;
        if (path.empty())
            filePath = path_;
        else
            filePath = fs::path(path);     
     
        if (fs::is_directory(filePath))
        {   
            // Search all xml files from path. 
            std::list<std::string> files = GetFiles(filePath);

            for (std::list<std::string>::iterator iter = files.begin(); iter != files.end(); ++iter)
            {
                std::string group = "";
                
                // Get group name (it is encoded to filename)

                fs::path file(*iter);
                
                std::string fileName = file.filename();
                std::string::size_type bPos = fileName.find(file_name_encoding_);
                
                if( bPos != std::string::npos)
                {
                    // File is valid. 
                    bPos +=file_name_encoding_.size();
                    std::string::size_type ePos = fileName.find(".xml");
                    // Get group name.
                    group = fileName.substr(bPos, ePos - bPos);
                }
                else
                {
                    // File was invalid. 
                    continue;
                }

              
               //! Poco xml configuration reader
               Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfiguration;
                
               try
                {
                   pConfiguration = new Poco::Util::XMLConfiguration();
                   std::fstream stream(iter->c_str(), std::ios::in);
                   pConfiguration->load(stream);
                } catch ( std::exception& /*ex*/)
                {
                    // XML file was not valid (not so beatiful way to check validity)
                    continue;
                }
                    
                if (pConfiguration.get() != 0)
                {
                    // Read values from xml file and set them into memory.
                    AddValues(pConfiguration, group);
                }
                
            }
        }
        else
        {
            // Path is a file. 
            
            std::string group = "";
            
            std::string fileName = filePath.filename();
            std::string::size_type bPos = fileName.find(file_name_encoding_);
                
            if( bPos != std::string::npos)
            {
                // File is valid. 
                
                std::string::size_type ePos = fileName.find(".xml");
                
                // Get group name.
                
                group = fileName.substr(bPos, ePos - bPos);
            }
            else
            {
                // File was invalid. 
                return;
            }

            Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfiguration;
            
            try
            {
                   pConfiguration = new Poco::Util::XMLConfiguration();
                   std::fstream stream(filePath.file_string().c_str(), std::ios::in);
                   pConfiguration->load(stream);
            } catch ( std::exception& /*ex*/)
            {
                // XML file was not valid (not so beatiful way to check it validity)
                
                // Create exception.
                std::string what = std::string("Failed to load configuration file: ") + path;
                throw Exception(what.c_str());
            }

            if ( pConfiguration.get() != 0 )
            {
                AddValues(pConfiguration,group);
             
            }
            
        }
    }
    
    std::list<std::string> ConfigurationManager::GetFiles(const boost::filesystem::path& path) const
    {
        namespace fs = boost::filesystem;

        std::list<std::string> files;
        fs::directory_iterator iterEnd;
       
        for ( fs::directory_iterator iter(path); iter != iterEnd; ++iter )
        {
            try
            {
                if ( fs::is_regular_file(iter->status()))
                {
                    // Check that filename contains .xml 
                    std::string file = iter->path().filename();
                    if ( file.find(".xml") != std::string::npos)
                        files.push_back(iter->path().file_string());

                }
            }
            catch (std::exception& /*ex*/)
            {
                // If exception is thrown, return fast. 
                return files;
            }
     
        }
        return files;
    }


    void ConfigurationManager::AddValues(Poco::Util::XMLConfiguration* pConfiguration, const std::string& group)
    {
        // Collect all key-value pairs from configuration and save them into data map.
        
        if (pConfiguration != 0)
        {
            
            std::vector<std::string> keys;
            
            // For more faster implementation reserve some memory space, magical value 100 element. 
            
            keys.reserve(100);
            pConfiguration->keys(group, keys);
            
            // Note we don't support multi level keys.  
             
            for ( std::vector<std::string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
            {
                // Add keys into map.
                try
                {
                    std::string key = group + "." + *iter;
                    std::string value = pConfiguration->getString(key);
                    SetSetting(group, *iter, value);
                
                } catch (std::exception& /*ex*/)
                {
                    ///todo How to handle errors, at the moment just continue.
                    continue;
                }
            }

        }
        
    }

    void ConfigurationManager::Export(const std::string& path, const std::string& group)
    {
                    
        namespace fs = boost::filesystem; 
        fs::path filePath;
        if (path.empty())
            filePath = path_;
        else
            filePath = fs::path(path);     
     
        
     
        // Check that given path is exist.        
        if ( !fs::exists(filePath))
        {
            ///todo Should we write here error message into Log if given path does not exist?         
            return;
        }
        
        if (group == "" && fs::is_directory(filePath))
        {
            // Export all values from memory map. 

            // Create temporary map which contains all data.

            std::map<string_pair_t, std::string> tmp_values = values_;

            // Get all xml files which are located in given path (does there exist earlier configurations)
            
            std::list<std::string> files = GetFiles(filePath);
        
            for ( std::list<std::string>::iterator file_iter = files.begin(); file_iter != files.end(); ++file_iter)
            {
                // Check that xml file match our encoding. 
                
                fs::path file(*file_iter);
                std::string fileName = file.filename();
                
                std::string::size_type bPos = fileName.find(file_name_encoding_);
                std::string search_group = "";
                
                if (bPos != std::string::npos)
                {
                    std::string::size_type ePos = fileName.find(".xml");
                    bPos += file_name_encoding_.size();
                    search_group = fileName.substr(bPos, ePos - bPos);
                } 
                else
                    continue;


                Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfiguration;
            
                try
                {
                   pConfiguration = new Poco::Util::XMLConfiguration();
                   std::fstream stream(file.file_string().c_str(), std::ios::in);
                   pConfiguration->load(stream);                    
                } catch ( std::exception& /*ex*/)
                {
                    // XML file was not valid (not so beatiful way to check it validity)
                    
                    ///todo What to do if loading failed. 
                    continue;
                }


                // Go through all runtime map values and set all values which match to group into file.
                
                std::map<string_pair_t, std::string>::iterator val_iter = values_.begin();
                while(val_iter != values_.end())
                {
                    std::map<string_pair_t, std::string>::iterator next = val_iter;
                    ++next;
                    
                    if ( val_iter->first.first == search_group)
                    {
                        // Save key. 
                        std::string key = search_group + "." + val_iter->first.second;
                 
                        // Corresponding value 
                        std::string value = val_iter->second;
                        pConfiguration->setString(key, value);
                        // Remove old value. 
                        values_.erase(val_iter);
                    }
                    val_iter = next;
                }
                
                // Now save it into file.
                {
                    std::fstream stream(file.file_string().c_str(), std::ios::out);
                    pConfiguration->save(stream);
                }
            }

            Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfiguration;
          
             // Write values which were not saved earlier.

            if ( values_.size() != 0)
            {
                std::map<string_pair_t, std::string>::iterator val_iter = values_.begin();
                
                while (val_iter != values_.end() )
                {
                    //std::map<string_pair_t, std::string>::iterator next_upper = val_iter;
                   

                    std::string search_group = val_iter->first.first;
                    
                    
                    /// \todo HACK assure that path end has separator.

                    std::string c = filePath.string().substr(filePath.string().size()-1);
                    if ( c != std::string("/") && c != std::string("\\"))
                    {
#ifdef _UNIX
                        filePath = fs::path(filePath.directory_string() + "\\");
#else
                        filePath = fs::path(filePath.directory_string() + "/");
#endif
                        
                    }
                    // END HACK
                    
                    fs::path new_configuration_file = filePath.directory_string() + file_name_encoding_ + search_group + ".xml";
                    
                    try
                    {
                        pConfiguration = new Poco::Util::XMLConfiguration;
                        pConfiguration->loadEmpty(std::string("config"));
                    } catch ( std::exception& /*ex*/)
                    {
                        ///todo What to do if loading failed. 
                        continue;
                    }

                    
                    // Go through all values which are still left and find all keys which belongs to given group. 

                    std::map<string_pair_t, std::string>::iterator val_key = values_.begin();
                    while(val_key != values_.end())
                    {
                        std::map<string_pair_t, std::string>::iterator next = val_key;
                        ++next;

                        if ( val_key->first.first == search_group)
                        {
                            // Save key
                            std::string key = search_group + "." + val_key->first.second;
                            // Corresponding value 
                            std::string value = val_key->second;
                            pConfiguration->setString(key, value);
                            // Remove old value. 
                            values_.erase(val_key);
                        }
                        val_key = next;
                    }
                  
                    {
                        std::fstream stream(new_configuration_file.file_string().c_str(), std::ios::out);
                        pConfiguration->save(stream);
                    }
                    
                    // HACK

                    if (values_.size() == 0)
                        val_iter = values_.end();
                    else
                        val_iter = values_.begin();
                    // END HACK

                   
                }

            }
            
            // Return values so that ConfigurationManager state is same as before.
            values_ = tmp_values;

         }
         else if (group != "" && fs::is_directory(filePath))
         {
            //Export only values from given param group into folder.
            
            // Search first that is there all ready file which match encoding. 
            
            // Create temporary map which contains all data.

            std::map<string_pair_t, std::string> tmp_values = values_;

            // Get all xml files which are located in given path (does there exist earlier configurations)
            
            std::list<std::string> files = GetFiles(filePath);

            bool found_file = false;

            for ( std::list<std::string>::iterator file_iter = files.begin(); file_iter != files.end(); ++file_iter)
            {
                // Check that xml file match our encoding. 
                
                fs::path file(*file_iter);
                std::string fileName = file.filename();
                
                std::string::size_type bPos = fileName.find(file_name_encoding_);
               
                std::string search_group = "";

                if (bPos != std::string::npos)
                {
                    std::string::size_type ePos = fileName.find(".xml");
                    bPos += file_name_encoding_.size();
                    search_group = fileName.substr(bPos, ePos - bPos);
                } 
                else
                    continue;

                if ( search_group != group )
                    continue;

                Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfiguration;
                found_file = true;

                try
                {
                   pConfiguration = new Poco::Util::XMLConfiguration();
                   std::fstream stream(file.file_string().c_str(), std::ios::in);
                   pConfiguration->load(stream);
                } catch ( std::exception& /*ex*/)
                {
                    // XML file was not valid (not so beatiful way to check it validity)
                    
                    ///todo What to do if loading failed. 
                    continue;
                }


                // Go through all runtime map values and set all values which match to group into file.
                
                std::map<string_pair_t, std::string>::iterator val_iter = values_.begin();
                while(val_iter != values_.end())
                {
                    std::map<string_pair_t, std::string>::iterator next = val_iter;
                    ++next;
                    
                    if ( val_iter->first.first == search_group)
                    {
                        // Save key. 
                        std::string key = search_group + "." + val_iter->first.second;
                 
                        // Corresponding value 
                        std::string value = val_iter->second;
                        pConfiguration->setString(key, value);
                        // Remove old value. 
                        values_.erase(val_iter);
                    }
                    val_iter = next;
                }
                
                // Now save it into file.
                {
                    std::fstream stream(file.file_string().c_str(), std::ios::out);
                    pConfiguration->save(stream);
                }
                
                // Break because we found a file.
                break;
            }

         
            if ( !found_file ) 
            {
                // There is not old configuration file.
                
                std::map<string_pair_t, std::string>::iterator val_iter = values_.begin();
                
                if(val_iter != values_.end() )
                {
                 

                    std::string search_group = group;
                    
                    
                    // HACK assure that path end has separator.

                    std::string c = path.substr(path.size()-1);
                    if ( c != std::string("/") && c != std::string("\\"))
                    {
#ifdef _UNIX
                        filePath = fs::path(filePath.directory_string() + "\\");
#else
                        filePath = fs::path(filePath.directory_string() + "/");
#endif
                        
                    }
                    // END HACK
                    
                    fs::path new_configuration_file = filePath.directory_string() + file_name_encoding_ + search_group + ".xml";
                    
                    Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfiguration;

                    try
                    {
                        pConfiguration = new Poco::Util::XMLConfiguration;
                        pConfiguration->loadEmpty(std::string("config"));
                    } catch ( std::exception& /*ex*/)
                    {
                        ///todo What to do if loading failed. 
                        values_ = tmp_values;
                        return;
                    }

                    
                    // Go through all values which are still left and find all keys which belongs to given group. 

                    std::map<string_pair_t, std::string>::iterator val_key = values_.begin();
                    while(val_key != values_.end())
                    {
                        std::map<string_pair_t, std::string>::iterator next = val_key;
                        ++next;

                        if ( val_key->first.first == search_group)
                        {
                            // Save key
                            std::string key = search_group + "." + val_key->first.second;
                            // Corresponding value 
                            std::string value = val_key->second;
                            pConfiguration->setString(key, value);
                            // Remove old value. 
                            values_.erase(val_key);
                        }
                        val_key = next;
                    }
                  
                    {
                        std::fstream stream(new_configuration_file.file_string().c_str(), std::ios::out);
                        pConfiguration->save(stream);
                    }
                                    
                }

            }

            values_ = tmp_values;

         }
         else if (group != "")
         {
            // Export data when given path is a file and group is diffrent then empty string.
            
            // Create temporary map which contains all data.

            std::map<string_pair_t, std::string> tmp_values = values_;
         
            std::map<string_pair_t, std::string>::iterator val_iter = values_.begin();
            
            if(val_iter != values_.end() )
            { 

                std::string search_group = group;
                                 
                fs::path new_configuration_file = filePath.directory_string() + file_name_encoding_ + search_group + ".xml";
                
                Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfiguration;

                try
                {
                    pConfiguration = new Poco::Util::XMLConfiguration;
                    pConfiguration->loadEmpty(std::string("config"));
                } catch ( std::exception& /*ex*/)
                {
                    ///todo What to do if loading failed. 
                    values_ = tmp_values;
                    return;
                }

                
                // Go through all values which are still left and find all keys which belongs to given group. 

                std::map<string_pair_t, std::string>::iterator val_key = values_.begin();
                while(val_key != values_.end())
                {
                    std::map<string_pair_t, std::string>::iterator next = val_key;
                    ++next;

                    if ( val_key->first.first == search_group)
                    {
                        // Save key
                        std::string key = search_group + "." + val_key->first.second;
                        // Corresponding value 
                        std::string value = val_key->second;
                        pConfiguration->setString(key, value);
                        // Remove old value. 
                        values_.erase(val_key);
                    }
                    val_key = next;
                }
          
                {
                    std::fstream stream(new_configuration_file.file_string().c_str(), std::ios::out);
                    pConfiguration->save(stream);
                }
                                
            }

            values_ = tmp_values;
         }
    }

    
   bool ConfigurationManager::HasKey(const std::string& group, const std::string& key) const
   {
        std::map<string_pair_t, std::string>::iterator iter = values_.find(std::make_pair(group, key));
        if ( iter != values_.end() )    
            return true;
        else
            return false;

   }

   void ConfigurationManager::Remove(const std::string& group, const std::string key)
   {
        std::map<string_pair_t, std::string>::iterator iter = values_.find(std::make_pair(group, key));
        if ( iter != values_.end() )    
            values_.erase(iter);
       
   }
}
