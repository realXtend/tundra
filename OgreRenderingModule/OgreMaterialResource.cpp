// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreMaterialResource.h"
#include "OgreRenderingModule.h"

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
            ///\todo Poor reading of the material name. Change this
            /// The Ogre material name and the OpenSim asset name should coincide, so we need to replace the name we read here with the
            /// Rex asset name.
            data->seek(0);
            char str[512] = {};
            char materialName[512] = {};
            data->readLine(str, 510);
            sscanf(str, "material %s", materialName);
            data->seek(0);

            Ogre::MaterialManager::getSingleton().parseScript(data, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Ogre::MaterialManager::getSingleton().create(materialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            ogre_material_ = Ogre::MaterialManager::getSingleton().getByName(materialName);

            if (ogre_material_->getName().length() == 0)
            {
                OgreRenderingModule::LogDebug(std::string("Warning: Possibly failed to create an ogre material from Rex Material asset ") +
                    source->GetId());
            }
            else
                OgreRenderingModule::LogDebug(std::string("Ogre material \"") + ogre_material_->getName() + "\" created from Rex Material asset " +
                    source->GetId());
        } catch (Ogre::Exception &e)
        {
            OgreRenderingModule::LogWarning(e.what());
            OgreRenderingModule::LogWarning("Failed to parse Ogre material " + id_ + ".");
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
}