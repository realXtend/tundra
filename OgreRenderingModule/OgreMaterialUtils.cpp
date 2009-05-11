// For conditions of distribution and use, see copyright notice in license.txt
/// @file OgreMaterialUtils.cpp
/// Contains some common methods for 

#include "StableHeaders.h"

#include "OgreMaterialUtils.h"

namespace OgreRenderer
{
    Ogre::MaterialPtr GetOrCreateUnlitTexturedMaterial(const char *materialName)
    {
        const char baseMaterialName[] = "UnlitTextured";

        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(materialName);

        if (!material.get())
        {
            Ogre::MaterialPtr baseMaterial = mm.getByName(baseMaterialName);
            material = baseMaterial->clone(materialName);
        }

        assert(material.get());
        return material;
    }
}
