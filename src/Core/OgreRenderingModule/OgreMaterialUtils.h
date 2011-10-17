/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreMaterialUtils.h
 *  @brief  Contains some often needed utlitity functions when dealing with OGRE material scripts.
 */

#pragma once

#include <OgreMaterial.h>
#include "CoreTypes.h"
#include "OgreModuleApi.h"

namespace OgreRenderer
{
    /// If string @c str contains space, wraps the string with double quotes.
    std::string OGRE_MODULE_API AddDoubleQuotesIfNecessary(const std::string &str);

    /// Desanitates asset references in Ogre scripts.
    /** @param script Ogre script data as string.
        @param keywords List of keywords/IDs <b> appended with a space </b>, e.g. "material ", "texture " and "particle_system ". */
    void OGRE_MODULE_API DesanitateAssetIds(std::string &script, const QStringList &keywords);

    /// Returns an Ogre material with the given name, or creates it if it doesn't exist.
    /** The material is derived from an UnlitTextured material, that's a simple one to use for debugging visualizations. */
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateUnlitTexturedMaterial(const std::string& materialName);

    /// Returns an Ogre material with the given name, or creates it if it doesn't exist.
    /** The material is derived from the LitTextured material. */
    Ogre::MaterialPtr OGRE_MODULE_API GetOrCreateLitTexturedMaterial(const std::string& materialName);

    /// Creates clone from material.
    Ogre::MaterialPtr OGRE_MODULE_API CloneMaterial(const std::string& sourceMaterialName, const std::string &newName);

    /// Sets texture unit on a material to a given texture name.
    /** If texture cannot actually be found, uses the missing texture texture */ 
    void OGRE_MODULE_API SetTextureUnitOnMaterial(Ogre::MaterialPtr material, const std::string& textureName, uint index = 0);

    /// Replaces texture name in all passes, techniques, textureunits with another name
    /** If replacement texture cannot actually be found, uses the missing texture texture */ 
    void OGRE_MODULE_API ReplaceTextureOnMaterial(Ogre::MaterialPtr material, const std::string& originalName, const std::string& textureName);

    /// Returns texture names used by a material's all techniques, passes & textureunits. Does not return duplicates.
    void OGRE_MODULE_API GetTextureNamesFromMaterial(Ogre::MaterialPtr material, StringVector& textures);

    /// Counts indentation levels of brace blocks in a file.
    bool OGRE_MODULE_API ProcessBraces(const std::string& line, int& braceLevel);
}
