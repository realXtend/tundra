// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreMaterialResource.h"
#include "OgreParticleResource.h"
#include "OgreTextureResource.h"
#include "OgreRenderingModule.h"
#include "OgreMaterialUtils.h"
#include "ResourceHandler.h"

#include <Ogre.h>

namespace OgreRenderer
{
    OgreParticleResource::OgreParticleResource(const std::string& id) : 
        ResourceInterface(id)
    {
    }

    OgreParticleResource::OgreParticleResource(const std::string& id, Foundation::AssetPtr source) : 
        ResourceInterface(id)
    {
        SetData(source);
    }

    OgreParticleResource::~OgreParticleResource()
    {
        RemoveTemplates();
    }

    bool OgreParticleResource::SetData(Foundation::AssetPtr source)
    {
        RemoveTemplates();
        references_.clear();

        if (!source)
        {
            OgreRenderingModule::LogError("Null source asset data pointer");     
            return false;
        }
        if (!source->GetSize())
        {
            OgreRenderingModule::LogError("Zero sized particle system asset");     
            return false;
        }

        // Detected template names
        Core::StringVector new_templates;
            
        Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(const_cast<Core::u8 *>(source->GetData()), source->GetSize()));
        try
        {
            int brace_level = 0;
            bool skip_until_next = false;
            int skip_brace_level = 0;
            // Parsed/modified script
            std::ostringstream output;

            while (!data->eof())
            {
                Ogre::String line = data->getLine();
                // Skip empty lines & comments
                if ((line.length()) && (line.substr(0, 2) != "//"))
                {
                    // Process opening/closing braces
                    if (!ResourceHandler::ProcessBraces(line, brace_level))
                    {
                        // If not a brace and on level 0, it should be a new particlesystem; replace name with resource ID + ordinal
                        if (brace_level == 0)
                        {
                            line = id_ + "_" + Core::ToString<size_t>(new_templates.size());
                            new_templates.push_back(line);
                            // New script compilers need this
                            line = "particle_system " + line;
                        }
                        else
                        {
                            // Check for ColourImage, which is a risky affector and may easily crash if image can't be loaded
                            if (line.substr(0, 8) == "affector")
                            {
			      std::vector<Ogre::String> line_vec;

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR == 6 
			      line_vec = Ogre::StringUtil::split(line, "\t ");
#else 
			      Ogre::vector<Ogre::String>::type vec = Ogre::StringUtil::split(line,"\t ");
			      int size = vec.size();
			      line_vec.resize(size);
			      
			      for (int i = 0; i < size; ++i)
				line_vec[i] = vec[i];
#endif                                
                               if (line_vec.size() >= 2)
                                {
                                    if (line_vec[1] == "ColourImage")
                                    {
                                        skip_until_next = true;
                                        skip_brace_level = brace_level;
                                    }
                                }
                            }
                            // Check for image/material definition
                            else if (line.substr(0, 8) == "material")
                            {
			      std::vector<Ogre::String> line_vec;
#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR == 6 
			      line_vec = Ogre::StringUtil::split(line, "\t ");
#else 
			      Ogre::vector<Ogre::String>::type vec = Ogre::StringUtil::split(line,"\t ");
			      int size = vec.size();
			      line_vec.resize(size);
			      
			      for (int i = 0; i < size; ++i)
				line_vec[i] = vec[i];
#endif                                         
                                if (line_vec.size() >= 2)
                                {
                                    std::string mat_name = line_vec[1];
                                    // Material script mode
                                    if ((line_vec.size() >= 3) && (line_vec[2].substr(0,6) == "script"))
                                    {
                                        references_.push_back(Foundation::ResourceReference(mat_name, OgreMaterialResource::GetTypeStatic()));
                                        line = "material " + mat_name;
                                    }
                                    // Texture mode
                                    else 
                                    {
                                        std::string variation;
                                        if (line_vec.size() >= 3)
                                            variation = line_vec[2];
                                        
                                        if (!IsMaterialSuffixValid(variation))
                                            variation = "";
                                            
                                        references_.push_back(Foundation::ResourceReference(mat_name, OgreTextureResource::GetTypeStatic()));
                                        line = "material " + mat_name + variation;
                                    }
                                }
                            }
                        }
                        // Write line to the copy
                        if (!skip_until_next)
                            output << line << std::endl;
                        else
                            OgreRenderingModule::LogDebug("Skipping risky particle effect line: " + line);
                    }
                    else
                    {
                        // Write line to the copy
                        if (!skip_until_next)
                            output << line << std::endl;
                        else
                            OgreRenderingModule::LogDebug("Skipping risky particle effect line: " + line);

                        if (brace_level <= skip_brace_level)
                            skip_until_next = false;
                    }
                } 
            }

            std::string output_str = output.str();
            Ogre::DataStreamPtr modified_data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(&output_str[0], output_str.size()));
            Ogre::ParticleSystemManager::getSingleton().parseScript(modified_data, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogWarning(e.what());
            OgreRenderingModule::LogWarning("Failed to parse Ogre particle script " + source->GetId() + ".");
        }
        
        // Check which templates actually succeeded
        for (Core::uint i = 0; i < new_templates.size(); ++i)
        {
            if (Ogre::ParticleSystemManager::getSingleton().getTemplate(new_templates[i]))
            {
                templates_.push_back(new_templates[i]);
                OgreRenderingModule::LogDebug("Ogre particle system template " + new_templates[i] + " created");
            }
        }
        
        // Theoretical success if at least one template was created
        return IsValid();
    }

    static const std::string type_name("OgreParticle");
        
    const std::string& OgreParticleResource::GetType() const
    {
        return type_name;
    }
    
    const std::string& OgreParticleResource::GetTypeStatic()
    {
        return type_name;
    }    
    
    Core::uint OgreParticleResource::GetNumTemplates() const
    {
        return templates_.size();
    }
    
    const std::string& OgreParticleResource::GetTemplateName(Core::uint index) const
    {
        static const std::string empty;
        if (index >= templates_.size())
            return empty;
            
        return templates_[index];
    }
    
    void OgreParticleResource::RemoveTemplates()
    {
        for (unsigned i = 0; i < templates_.size(); ++i)
        {
            try
            {
                Ogre::ParticleSystemManager::getSingleton().removeTemplate(templates_[i]);
            } catch (...) {}
        }
        templates_.clear();
    }
    
    bool OgreParticleResource::IsValid() const
    {
        return (templates_.size() > 0);
    }
}
