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
    
    //! Sets texture unit on a material to a given texture name. Use index SET_ALL_UNITS to set all units.
    void OGRE_MODULE_API SetTextureUnitOnMaterial(Ogre::MaterialPtr material, Core::uint index, const char *textureName);
}

#endif
