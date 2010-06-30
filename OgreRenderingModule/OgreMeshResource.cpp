// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreMeshResource.h"
#include "OgreRenderingModule.h"

#include <Ogre.h>

namespace OgreRenderer
{
    OgreMeshResource::OgreMeshResource(const std::string& id) : 
        ResourceInterface(id)
    {
    }

    OgreMeshResource::OgreMeshResource(const std::string& id, Foundation::AssetPtr source) : 
        ResourceInterface(id)
    {
        SetData(source);
    }

    OgreMeshResource::~OgreMeshResource()
    {
        RemoveMesh();
    }

    bool OgreMeshResource::SetData(Foundation::AssetPtr source)
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
                ogre_mesh_->setAutoBuildEdgeLists(false);
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
            
            // Generate extremity points to submeshes, 1 should be enough
            try
            {
                for(uint i = 0; i < ogre_mesh_->getNumSubMeshes(); ++i)
                {
                    Ogre::SubMesh *smesh = ogre_mesh_->getSubMesh(i);
                    if (smesh)
                        smesh->generateExtremes(1);
                }
            }
            catch (...) {}
            
            // Assign default materials that won't complain
            original_materials_.clear();
            for (uint i = 0; i < ogre_mesh_->getNumSubMeshes(); ++i)
            {
                Ogre::SubMesh* submesh = ogre_mesh_->getSubMesh(i);
                if (submesh)
                {
                    original_materials_.push_back(submesh->getMaterialName());
                    submesh->setMaterialName("LitTextured");
                }
            }
            
        }
        catch (Ogre::Exception &e)
        {
            OgreRenderingModule::LogError("Failed to create mesh " + id_ + ": " + std::string(e.what()));
            RemoveMesh();
            return false;
        }

        OgreRenderingModule::LogDebug("Ogre mesh " + id_ + " created");
        return true;
    }

    static const std::string type_name("OgreMesh");
        
    const std::string& OgreMeshResource::GetType() const
    {
        return type_name;
    }
    
    const std::string& OgreMeshResource::GetTypeStatic()
    {
        return type_name;
    }    
    
    void OgreMeshResource::RemoveMesh()
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
    
    bool OgreMeshResource::IsValid() const
    {
        return (!ogre_mesh_.isNull());
    }
}