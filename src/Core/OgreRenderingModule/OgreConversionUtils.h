/** 
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreConversionUtils.h
 *  @brief  Contains some common methods for conversions between Ogre and Core variable types.
 */

#pragma once

#include "OgreModuleApi.h"

namespace OgreRenderer
{
    /// If string @c str contains space, wraps the string with double quotes.
    std::string OGRE_MODULE_API AddDoubleQuotesIfNecessary(const std::string &str);

    /// Desanitates asset references in Ogre scripts.
    /** @param script Ogre script data as string.
        @param keywords List of keywords/IDs <b> appended with a space </b>, e.g. "material ", "texture " and "particle_system ". */
    void OGRE_MODULE_API DesanitateAssetIds(std::string &script, const QStringList &keywords);
}

