// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreLocalResourceUtils.h"
#include "OgreRenderingModule.h"
#include <Ogre.h>

namespace OgreRenderer
{
    Ogre::MeshPtr GetLocalMesh(const std::string& name)
    {
        Ogre::MeshManager& manager = Ogre::MeshManager::getSingleton();
        
        Ogre::MeshPtr mesh = manager.getByName(name);
        if (mesh.isNull())
        {
            try
            {
                manager.load(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                mesh = manager.getByName(name);
            }
            catch (...) {}
        }
        
        return mesh;
    }
    
    Ogre::SkeletonPtr GetLocalSkeleton(const std::string& name)
    {
        Ogre::SkeletonManager& manager = Ogre::SkeletonManager::getSingleton();
        
        Ogre::SkeletonPtr skel = manager.getByName(name);
        if (skel.isNull())
        {
            try
            {
                manager.load(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                skel = manager.getByName(name);
            }
            catch (...) {}
        }
        
        return skel;
    }
    
    Ogre::MaterialPtr GetLocalMaterial(const std::string& name)
    {
        Ogre::MaterialManager& manager = Ogre::MaterialManager::getSingleton();
        
        Ogre::MaterialPtr mat = manager.getByName(name);
        if (mat.isNull())
        {
            try
            {
                manager.load(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                mat = manager.getByName(name);
            }
            catch (...) {}
        }
        
        return mat;
    }
    
    Ogre::TexturePtr GetLocalTexture(const std::string& name)
    {
        Ogre::TextureManager& manager = Ogre::TextureManager::getSingleton();
        
        Ogre::TexturePtr tex = manager.getByName(name);
        if (tex.isNull())
        {
            try
            {
                manager.load(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                tex = manager.getByName(name);
            }
            catch (...) {}
        }
        
        return tex;
    }
}