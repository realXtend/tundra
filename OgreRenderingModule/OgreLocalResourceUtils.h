#ifndef incl_OgreLocalResourceUtils_h
#define incl_OgreLocalResourceUtils_h

#include <OgreMesh.h>
#include <OgreSkeleton.h>
#include <OgreMaterial.h>
#include <OgreTexture.h>

#include "OgreModuleApi.h"

namespace OgreRenderer
{
    // Returns a local mesh resource, or null if cannot be found/loaded
    Ogre::MeshPtr OGRE_MODULE_API GetLocalMesh(const std::string& name);
    
    // Returns a local skeleton resource, or null if cannot be found/loaded
    Ogre::SkeletonPtr OGRE_MODULE_API GetLocalSkeleton(const std::string& name);
    
    // Returns a local material resource, or null if cannot be found/loaded
    Ogre::MaterialPtr OGRE_MODULE_API GetLocalMaterial(const std::string& name);    
    
    // Returns a local texture resource, or null if cannot be found/loaded
    Ogre::TexturePtr OGRE_MODULE_API GetLocalTexture(const std::string& name);
};

#endif
