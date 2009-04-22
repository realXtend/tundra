// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreMesh.h"
#include "OgreRenderingModule.h"

#include <Ogre.h>

namespace OgreRenderer
{
    OgreMesh::OgreMesh(const std::string& id) : 
        ResourceInterface(id)
    {
    }

    OgreMesh::OgreMesh(const std::string& id, Foundation::AssetPtr source) : 
        ResourceInterface(id)
    {
        SetData(source);
    }

    OgreMesh::~OgreMesh()
    {
        if (!ogre_mesh_.isNull())
        {
            std::string mesh_name = ogre_mesh_->getName();
            ogre_mesh_.setNull();

            try
            {
                Ogre::MeshManager::getSingleton().remove(mesh_name);
            }
            catch (...) {}
        }
    }

    bool OgreMesh::SetData(Foundation::AssetPtr source)
    {
        if (!source)
        {
            OgreRenderingModule::LogError("Null source asset data pointer");     
            return false;
        }
        if (!source->GetSize())
        {
            OgreRenderingModule::LogError("Zero sized mesh asset");     
            return false;
        }
                
        try
        {
            if (ogre_mesh_.isNull())
            {   
                ogre_mesh_ = Ogre::MeshManager::getSingleton().createManual(
                    id_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

                if (ogre_mesh_.isNull())
                {
                    OgreRenderingModule::LogError("Failed to create mesh " + id_);
                    return false; 
                }   
            }

            Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)source->GetData(), source->GetSize(), false));
            Ogre::MeshSerializer serializer;
            serializer.importMesh(stream, ogre_mesh_.getPointer());

            // Generate tangents to mesh
            try
            {
                unsigned short src, dest;
                if (!ogre_mesh_->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
                    ogre_mesh_->buildTangentVectors(Ogre::VES_TANGENT, src, dest);
            }
            catch (...) {}
        }
        catch (Ogre::Exception &e)
        {
            OgreRenderingModule::LogError("Failed to create mesh " + id_ + ": " + std::string(e.what()));
            return false;
        }

        OgreRenderingModule::LogInfo("Ogre mesh " + id_ + " created");
        return true;
    }

    const std::string& OgreMesh::GetTypeName() const
    {
        static const std::string name("OgreMesh");

        return name;
    }
}