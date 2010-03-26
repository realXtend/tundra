// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreSkeletonResource.h"
#include "OgreRenderingModule.h"

#include <Ogre.h>

namespace OgreRenderer
{
    OgreSkeletonResource::OgreSkeletonResource(const std::string& id) :
        ResourceInterface(id)
    {
    }

    OgreSkeletonResource::OgreSkeletonResource(const std::string& id, Foundation::AssetPtr source) :
        ResourceInterface(id)
    {
        SetData(source);
    }

    OgreSkeletonResource::~OgreSkeletonResource()
    {
        RemoveSkeleton();
    }

    bool OgreSkeletonResource::SetData(Foundation::AssetPtr source)
    {
        if (!source)
        {
            OgreRenderingModule::LogError("Null source asset data pointer");
            return false;
        }
        if (!source->GetSize())
        {
            OgreRenderingModule::LogError("Zero sized skeleton asset");
            return false;
        }

        try
        {
            if (ogre_skeleton_.isNull())
            {
                ogre_skeleton_ = Ogre::SkeletonManager::getSingleton().create(
                    id_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

                if (ogre_skeleton_.isNull())
                {
                    OgreRenderingModule::LogError("Failed to create skeleton " + id_);
                    return false; 
                }
            }

            Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)source->GetData(), source->GetSize(), false));
            Ogre::SkeletonSerializer serializer;
            serializer.importSkeleton(stream, ogre_skeleton_.getPointer());
        }
        catch (Ogre::Exception &e)
        {
            OgreRenderingModule::LogError("Failed to create skeleton " + id_ + ": " + std::string(e.what()));
            RemoveSkeleton();
            return false;
        }

        OgreRenderingModule::LogDebug("Ogre skeleton " + id_ + " created");
        return true;
    }

    static const std::string type_name("OgreSkeleton");

    const std::string& OgreSkeletonResource::GetType() const
    {
        return type_name;
    }

    const std::string& OgreSkeletonResource::GetTypeStatic()
    {
        return type_name;
    }

    void OgreSkeletonResource::RemoveSkeleton()
    {
        if (!ogre_skeleton_.isNull())
        {
            std::string skeleton_name = ogre_skeleton_->getName();
            ogre_skeleton_.setNull();

            try
            {
                Ogre::SkeletonManager::getSingleton().remove(skeleton_name);
            }
            catch (...) {}
        }
    }

    bool OgreSkeletonResource::IsValid() const
    {
        return (!ogre_skeleton_.isNull());
    }
}
