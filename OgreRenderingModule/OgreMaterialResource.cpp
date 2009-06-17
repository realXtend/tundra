// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreMaterialResource.h"
#include "OgreTextureResource.h"
#include "OgreRenderingModule.h"
#include "ResourceHandler.h"

#include <Ogre.h>

namespace OgreRenderer
{
    OgreMaterialResource::OgreMaterialResource(const std::string& id) : 
        ResourceInterface(id)
    {
    }

    OgreMaterialResource::OgreMaterialResource(const std::string& id, Foundation::AssetPtr source) : 
        ResourceInterface(id)
    {
        SetData(source);
    }

    OgreMaterialResource::~OgreMaterialResource()
    {
        RemoveMaterial();
    }

    bool OgreMaterialResource::SetData(Foundation::AssetPtr source)
    {
        // Remove old material if any
        RemoveMaterial();
        references_.clear();

        OgreRenderingModule::LogDebug("Parsing material " + source->GetId());
        
        if (!source)
        {
            OgreRenderingModule::LogError("Null source asset data pointer");
            return false;
        }
        if (!source->GetSize())
        {
            OgreRenderingModule::LogError("Zero sized material asset");
            return false;
        }

        Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(const_cast<Core::u8 *>(source->GetData()), source->GetSize()));
        try
        {
            int num_materials = 0;
            int brace_level = 0;
            bool skip_until_next = false;
            int skip_brace_level = 0;
            // Parsed/modified material script
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
                        // If not a brace and on level 0, it should be a new material; replace name
                        if ((brace_level == 0) && (line.substr(0, 8) == "material"))
                        {
                            if (num_materials == 0)
                            {
                                line = "material " + id_;
                                ++num_materials;
                            }
                            else
                            {
                                OgreRenderingModule::LogWarning("More than one material defined in material asset " + source->GetId() + " - only first one supported");
                                break;
                            }
                        }
                        else
                        {
                            // Check for textures
                            if ((line.substr(0, 8) == "texture ") && (line.length() > 8))
                            {
                                std::string tex_name = line.substr(8);
                                // Note: we assume all texture references are asset based. ResourceHandler checks later whether this is true,
                                // before requesting the reference
                                references_.push_back(Foundation::ResourceReference(tex_name, OgreTextureResource::GetTypeStatic()));
                            }
                        }

                        // Write line to the modified copy
                        if (!skip_until_next)
                            output << line << std::endl;
                    }
                    else
                    {
                        // Write line to the modified copy
                        if (!skip_until_next)
                            output << line << std::endl;

                        if (brace_level <= skip_brace_level)
                            skip_until_next = false;
                    }
                }
            }

            /////\todo Poor reading of the material name. Change this
            ///// The Ogre material name and the OpenSim asset name should coincide, so we need to replace the name we read here with the
            ///// Rex asset name.
            //data->seek(0);
            //char str[512] = {};
            //char materialName[512] = {};
            //data->readLine(str, 510);
            //sscanf(str, "material %s", materialName);
            //data->seek(0);

            std::string output_str = output.str();
            Ogre::DataStreamPtr modified_data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream((Core::u8 *)(&output_str[0]), output_str.size()));

            Ogre::MaterialManager::getSingleton().parseScript(modified_data, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);            
            ogre_material_ = Ogre::MaterialManager::getSingleton().getByName(id_);
            if (ogre_material_.isNull())
            {
                OgreRenderingModule::LogWarning(std::string("Failed to create an Ogre material from Rex Material asset ") +
                    source->GetId());

                return false;
            }
        } catch (Ogre::Exception &e)
        {
            OgreRenderingModule::LogWarning(e.what());
            OgreRenderingModule::LogWarning("Failed to parse Ogre material " + source->GetId() + ".");

            ogre_material_ = Ogre::MaterialManager::getSingleton().getByName(id_);
	    Ogre::ResourcePtr ogre_material = static_cast<Ogre::ResourcePtr>(ogre_material_);
	    //Ogre::MaterialManager::getSingleton().remove(static_cast<Ogre::ResourcePtr>(ogre_material_));
	    Ogre::MaterialManager::getSingleton().remove(ogre_material);
            return false;
        }
        return true;
    }

    static const std::string type_name("OgreMaterial");
        
    const std::string& OgreMaterialResource::GetType() const
    {
        return type_name;
    }
    
    const std::string& OgreMaterialResource::GetTypeStatic()
    {
        return type_name;
    }    
    
    void OgreMaterialResource::RemoveMaterial()
    {
        if (!ogre_material_.isNull())
        {
            std::string material_name = ogre_material_->getName();
            ogre_material_.setNull();

            try
            {
                Ogre::MaterialManager::getSingleton().remove(material_name);
            }
            catch (...)
            {
                OgreRenderingModule::LogDebug("Warning: Ogre::MaterialManager::getSingleton().remove(material_name); failed in OgreMaterialResource.cpp.");
            }
        }
    }
    
    bool OgreMaterialResource::IsValid() const
    {
        return (!ogre_material_.isNull());
    }
}
