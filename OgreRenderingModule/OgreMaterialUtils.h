#ifndef incl_OgreMaterialUtils_h
#define incl_OgreMaterialUtils_h

#include <Ogre.h>

#include "OgreModuleApi.h"

namespace OgreRenderer
{
    //! Standard legacy variation (lit, hard alpha if any)
    const Core::uint MAT_NORMAL = 0;
    
    //! Fullbright legacy variation (can be combined with others)
    const Core::uint MAT_FULLBRIGHT = 1;
    
    //! Soft alpha blended legacy variation
    const Core::uint MAT_ALPHA = 2;
    
    //! Additive blended legacy variation
    const Core::uint MAT_ADDITIVE = 4;
    
    //! Maximum legacy material variations
    const Core::uint MAX_MATERIAL_VARIATIONS = 6;

    //! Gets material suffix by variation type
    const std::string& GetMaterialSuffix(Core::uint variation);
    
    //! Returns if material suffix valid
    bool IsMaterialSuffixValid(const std::string& suffix);
    
    /// Returns an Ogre material with the given name, or creates it if it doesn't exist. The material
    /// is derived from an UnlitTextured material, that's a simple one to use for debugging visualizations.
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateUnlitTexturedMaterial(const char *materialName);

    /// Returns an Ogre material with the given name, or creates it if it doesn't exist. The material
    /// is derived from an LitTextured material, that's a simple one to use for basic diffuse texture visualizations.
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateLitTexturedMaterial(const char *materialName);

    //! Creates legacy material variations from texture
    void OGRE_MODULE_API CreateLegacyMaterials(const std::string& texture_name);

    //! Sets texture unit on a material to a given texture name.
    /*! If texture cannot actually be found, uses the missing texture texture
     */ 
    void OGRE_MODULE_API SetTextureUnitOnMaterial(Ogre::MaterialPtr material, const std::string& texture_name, Core::uint index = 0);
}

#endif
