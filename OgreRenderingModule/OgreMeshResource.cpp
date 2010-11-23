// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreMeshResource.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"
#include "Profiler.h"
#ifdef ASSIMP_ENABLED
#include "OpenAssetImport.h"
#endif

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
        PROFILE(OgreMeshResource_SetData);
        if (!source)
        {
            OgreRenderingModule::LogError("Null source asset data pointer");     
            return false;
        }
        
        if (!source->GetSize())
        {
            QString meshid = QString(id_.c_str());
            if (meshid.startsWith("mesh://", Qt::CaseInsensitive) && Ogre::MeshManager::getSingleton().resourceExists(SanitateAssetIdForOgre(id_)))
            {
                SetDefaultMaterial();
                return true;
            }

            OgreRenderingModule::LogError("Zero sized mesh asset");     
            return false;
        }
                
        try
        {
#ifdef ASSIMP_ENABLED
            // Check first if asset can be imported with Open Asset Import Library. If not, fall back to default Ogre mesh importer.
            // For efficiency we might also just check if id_ has .mesh extension and load it with Ogre importer, and only then check
            // if it can be imported with Assimp.
            AssImp::OpenAssetImport import;

            QString nodename;
            QString filepath;
            import.StripMeshnameFromAssetId(QString(id_.c_str()), filepath, nodename);
            if (import.IsSupportedExtension(filepath))
            {
                boost::filesystem::path path(filepath.toStdString());
                QString extension = QString(path.extension().c_str()).toLower();

                std::vector<std::string> importedMeshes;
                import.Import((void*)source->GetData(), source->GetSize(), QString(SanitateAssetIdForOgre(id_).c_str()), extension.toLatin1(), nodename, importedMeshes);
            } else
            {
#endif
                if (ogre_mesh_.isNull())
                {   
                    ogre_mesh_ = Ogre::MeshManager::getSingleton().createManual(
                        SanitateAssetIdForOgre(id_), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
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
#ifdef ASSIMP_ENABLED
            }
#endif
            
            // Assign default materials that won't complain
            SetDefaultMaterial();
            // Set asset references the mesh has
            ResetReferences();
        }
        catch (Ogre::Exception &e)
        {
            OgreRenderingModule::LogError("Failed to create mesh " + id_ + ": " + std::string(e.what()));
            RemoveMesh();
            return false;
        }

        internal_name_ = SanitateAssetIdForOgre(id_);
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

    void OgreMeshResource::SetDefaultMaterial()
    {
        if (!ogre_mesh_.isNull())
        {
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
    }

    void OgreMeshResource::ResetReferences()
    {
        references_.clear();
        if (!ogre_mesh_.isNull())
        {
            for (int i=0 ; i<ogre_mesh_->getNumSubMeshes() ; ++i)
            {
                Ogre::SubMesh *submesh = ogre_mesh_->getSubMesh(i);
                references_.push_back(Foundation::ResourceReference(submesh->getMaterialName(), "OgreMaterial"));
            }
        }
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

    bool OgreMeshResource::Export(const std::string &filename) const
    {
        if (ogre_mesh_.isNull())
        {
            OgreRenderingModule::LogWarning("Tried to export non-existing Ogre mesh " + id_ + ".");
            return false;
        }
        try
        {
            Ogre::MeshSerializer serializer;
            serializer.exportMesh(ogre_mesh_.get(), filename);
        } catch (std::exception &e)
        {
            OgreRenderingModule::LogError("Failed to export Ogre mesh " + id_ + ":");
            if (e.what())
                OgreRenderingModule::LogError(e.what());
            return false;
        }
        return true;
    }
}