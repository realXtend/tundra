/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreMaterialUtils.h
 *  @brief  Contains some often needed utlitity functions when dealing with OGRE material scripts.
 */

#ifndef incl_OgreRenderingModule_OgreMaterialUtils_h
#define incl_OgreRenderingModule_OgreMaterialUtils_h

#include <OgreMaterial.h>

#include "OgreModuleApi.h"
#include "ResourceInterface.h"

namespace OgreRenderer
{
    //! Standard legacy variation (lit)
    const uint LEGACYMAT_NORMAL = 0;
    
    //! Fullbright legacy variation (can be combined with others)
    const uint LEGACYMAT_FULLBRIGHT = 1;
    
    //! Additive blended legacy variation
    const uint LEGACYMAT_ADDITIVE = 2;
    
    //! Forced alpha legacy variation
    const uint LEGACYMAT_ALPHA = 4;

    //! Vertex color variation
    const uint LEGACYMAT_VERTEXCOL = 6;
    
    //! Vertex color + forced alpha variation
    const uint LEGACYMAT_VERTEXCOLALPHA = 8;
    
    //! Maximum legacy material variations
    const uint MAX_MATERIAL_VARIATIONS = 10;

    //! Gets material suffix by variation type
    std::string OGRE_MODULE_API GetMaterialSuffix(uint variation);
    
    //! Gets variation type by material suffix
    uint GetMaterialVariation(const std::string& suffix);
    
    //! Returns if material suffix valid
    bool OGRE_MODULE_API IsMaterialSuffixValid(const std::string& suffix);
    
    /// Returns an Ogre material with the given name, or creates it if it doesn't exist. The material
    /// is derived from an UnlitTextured material, that's a simple one to use for debugging visualizations.
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateUnlitTexturedMaterial(const std::string& materialName);

    /// Returns an Ogre material with the given name, or creates it if it doesn't exist. The material
    /// is derived from the LitTextured material.
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateLitTexturedMaterial(const std::string& materialName);

    Ogre::MaterialPtr OGRE_MODULE_API CloneMaterial(const std::string& sourceMaterialName, const std::string &newName);

    //! Create a legacy material variation, if it does not yet exist
    /*! @param texture_name texture to use
        @param variation Legacy variation type
     */
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateLegacyMaterial(const std::string& textureName, uint variation);
    
    //! Create a legacy material variation, if it does not yet exist
    /*! @param texture_name texture to use
        @param suffix Legacy variation type as a string suffix
     */
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateLegacyMaterial(const std::string& textureName, const std::string& suffix);
    
    //! Update existing legacy material variations from texture
    /*! @param texture_name texture to use
     */
    void OGRE_MODULE_API UpdateLegacyMaterials(const std::string& texture_name);

    //! Sets texture unit on a material to a given texture name.
    /*! If texture cannot actually be found, uses the missing texture texture
     */ 
    void OGRE_MODULE_API SetTextureUnitOnMaterial(Ogre::MaterialPtr material, const std::string& texture_name, uint index = 0);
    
    //! Replaces texture name in all passes, techniques, textureunits with another name
    /*! If replacement texture cannot actually be found, uses the missing texture texture
     */ 
    void OGRE_MODULE_API ReplaceTextureOnMaterial(Ogre::MaterialPtr material, const std::string& original_name, const std::string& texture_name);
    
    //! Returns texture names used by a material's all techniques, passes & textureunits. Does not return duplicates.
    void OGRE_MODULE_API GetTextureNamesFromMaterial(Ogre::MaterialPtr material, StringVector& textures);
    
    //! Deletes a material. Note: the material pointer passed in will be set to null
    void OGRE_MODULE_API RemoveMaterial(Ogre::MaterialPtr& material);

    /// Clones a new Ogre material that renders using the given ambient color. 
    /// This function will be removed or refactored later on, once proper material system is present. -jj.
    void OGRE_MODULE_API DebugCreateAmbientColorMaterial(const std::string &materialName, float r, float g, float b);
}

#endif
