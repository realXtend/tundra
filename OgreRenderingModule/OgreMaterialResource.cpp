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
            Ogre::MaterialManager::getSingleton().parseScript(data, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Ogre::MaterialManager::getSingleton().create(source->GetId(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            OgreRenderingModule::LogDebug("Ogre material " + id_ + " created");
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