#ifndef incl_OgreMaterialUtils_h
#define incl_OgreMaterialUtils_h

#include <Ogre.h>

#include "OgreModuleApi.h"

namespace OgreRenderer
{
    const Core::uint SET_ALL_UNITS = 0xFFFFFFFF;
    
    /// Returns an Ogre material with the given name, or creates it if it doesn't exist. The material
    /// is derived from an UnlitTextured material, that's a simple one to use for debugging visualizations.
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateUnlitTexturedMaterial(const char *materialName);

    /// Returns an Ogre material with the given name, or creates it if it doesn't exist. The material
    /// is derived from an LitTextured material, that's a simple one to use for basic diffuse texture visualizations.
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateLitTexturedMaterial(const char *materialName);
    
    //! Sets texture unit on a material to a given texture name. Use index SET_ALL_UNITS to set all units.
    /*! If texture cannot actually be found, uses the missing texture texture
     */ 
    void OGRE_MODULE_API SetTextureUnitOnMaterial(Ogre::MaterialPtr material, const char *textureName, Core::uint index = 0);
}

#endif
