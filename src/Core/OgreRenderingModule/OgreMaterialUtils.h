/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   OgreMaterialUtils.h
    @brief  Contains some often needed utlitity functions when dealing with OGRE material scripts. */

#pragma once

#include <OgreMaterial.h>
#include "CoreTypes.h"
#include "OgreModuleApi.h"

namespace OgreRenderer
{
    /// Stores information about material script.
    struct MaterialInfo
    {
        QString source; ///< Source file of the material script.
        QString name; ///< Name of the material.
        QString data; ///< Data (the actual material script).

        /// Less than operator. Compares source and name.
        bool operator <(const MaterialInfo &rhs) const
        {
            if (source < rhs.source) return true; else if (source > rhs.source) return false;
            if (name < rhs.name) return true; else if (name > rhs.name) return false;
            return false;
        }
    };

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

    /// Process a material file, searching for used materials and recording used textures
    /** @param matfilename Material file name, including full path
        @param used_materials Set of materials that are needed. Any materials in the file that are not listed will be skipped
        @return Set of used textures */
    QSet<QString> OGRE_MODULE_API ProcessMaterialFileForTextures(const QString& matfilename, const QSet<QString>& used_materials);

    /// Process material script and searches for texture references.
    /** @param material Material script.
        @return Set of used texture references/names. */
    QSet<QString> OGRE_MODULE_API ProcessMaterialForTextures(const QString &material);

    /// Loads single material script from material file and returns it as a string.
    /** @param filename Filename.
        @param materialName Material name.
        @return Material script as a string, or an empty string if material was not found. */
    QString OGRE_MODULE_API LoadSingleMaterialFromFile(const QString &filename, const QString &materialName);

    /// Loads all (uniquely named) material scripts found within a material script file.
    /** @param filename Filename.
        @param materialNames Names of materials to be loaded.
        @return List of material names - material script pairs as strings. */
    std::set<MaterialInfo> OGRE_MODULE_API LoadAllMaterialsFromFile(const QString &filename);

    /// Searches directory recursively and returns list of found material files.
    /** @param dir Directory to be searched. */
    QStringList OGRE_MODULE_API FindMaterialFiles(const QString &dir);
}
