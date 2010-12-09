#include "StableHeaders.h"
#include "OgreMeshAsset.h"
#include "OgreConversionUtils.h"
#include "OgreRenderingModule.h"

#include <Ogre.h>

bool OgreMeshAsset::DeserializeFromData(const u8 *data_, size_t numBytes)
{
    PROFILE(OgreMeshAsset_LoadFromFileInMemory);
    assert(data_);
    if (!data_)
        return false;
    
    if (ogreMesh.isNull())
    {   
        ogreMesh = Ogre::MeshManager::getSingleton().createManual(
            OgreRenderer::SanitateAssetIdForOgre(this->Name().toStdString()), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (ogreMesh.isNull())
        {
            OgreRenderer::OgreRenderingModule::LogError("Failed to create mesh " + Name().toStdString());
            return false; 
        }
        ogreMesh->setAutoBuildEdgeLists(false);
    }

    std::vector<u8> tempData(data_, data_ + numBytes);
    Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)&tempData[0], numBytes, false));
    Ogre::MeshSerializer serializer;
    serializer.importMesh(stream, ogreMesh.getPointer());
    
    // Generate tangents to mesh
    try
    {
        unsigned short src, dest;
        if (!ogreMesh->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
            ogreMesh->buildTangentVectors(Ogre::VES_TANGENT, src, dest);
    }
    catch (...) {}
    
    // Generate extremity points to submeshes, 1 should be enough
    try
    {
        for(uint i = 0; i < ogreMesh->getNumSubMeshes(); ++i)
        {
            Ogre::SubMesh *smesh = ogreMesh->getSubMesh(i);
            if (smesh)
                smesh->generateExtremes(1);
        }
    }
    catch (...) {}
        
    try
    {
        // Assign default materials that won't complain
        SetDefaultMaterial();
        // Set asset references the mesh has
//        ResetReferences();
    }
    catch (Ogre::Exception &e)
    {
        OgreRenderer::OgreRenderingModule::LogError("Failed to create mesh " + this->Name().toStdString() + ": " + std::string(e.what()));
        Unload();
        return false;
    }

//    internal_name_ = SanitateAssetIdForOgre(id_);
    OgreRenderer::OgreRenderingModule::LogDebug("Ogre mesh " + this->Name().toStdString() + " created");
    return true;
}

void OgreMeshAsset::Unload()
{
    if (ogreMesh.isNull())
        return;

    std::string meshName = ogreMesh->getName();
    ogreMesh.setNull();
    try
    {
        Ogre::MeshManager::getSingleton().remove(meshName);
    }
    catch (...) {}
}

void OgreMeshAsset::SetDefaultMaterial()
{
    if (ogreMesh.isNull())
        return;

//    originalMaterials.clear();
    for (uint i = 0; i < ogreMesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh *submesh = ogreMesh->getSubMesh(i);
        if (submesh)
        {
//            originalMaterials.push_back(submesh->getMaterialName().c_str());
            submesh->setMaterialName("LitTextured");
        }
    }
}
