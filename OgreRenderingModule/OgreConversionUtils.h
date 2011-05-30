/** 
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreConversionUtils.h
 *  @brief  Contains some common methods for conversions between Ogre and Core variable types.
 */

#pragma once

#include "Color.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "OgreModuleApi.h"

#include <OgreColourValue.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

namespace OgreRenderer
{
    /// Converts Color to Ogre::ColourValue.
    Ogre::ColourValue OGRE_MODULE_API ToOgreColor(const Color &color);

    /// Converts Ogre::ColourValue to Color.
    Color OGRE_MODULE_API ToCoreColor(const Ogre::ColourValue &color);

    /// Converts Vector3df to Ogre::Vector3.
    Ogre::Vector3 OGRE_MODULE_API ToOgreVector3(const Vector3df &vector);

    /// Converts Ogre::Vector3 to Vector3df.
    Vector3df OGRE_MODULE_API ToCoreVector(const Ogre::Vector3&vector);

    /// Converts Quaternion to Ogre::Quaternion
    Ogre::Quaternion OGRE_MODULE_API ToOgreQuaternion(const Quaternion &quat);

    /// Sanitates an asset URL/ID for use with Ogre's parsers.
    /** Call this when you access an Ogre resource directly from Ogre, by querying for asset ID.
        Replaces '/' with "$1" and ':' with "$2". If @c input contains '$', it's returned as is.
        @note This process is one-way only. If an asset depends on assets, the original asset ID's, not sanitated one's,
            should be recorded and used to request other the depended upon assets. */
    std::string OGRE_MODULE_API SanitateAssetIdForOgre(const QString& input);
    /// This is an overloaded function.
    std::string OGRE_MODULE_API SanitateAssetIdForOgre(const std::string& input);
    /// This is an overloaded function.
    std::string OGRE_MODULE_API SanitateAssetIdForOgre(const char* input);

    /// Desanitates an asset URL/ID for use with Ogre's parsers.
    QString OGRE_MODULE_API DesanitateAssetIdFromOgre(const QString &input);
    /// This is an overloaded function.
    QString OGRE_MODULE_API DesanitateAssetIdFromOgre(const std::string &input);

    /// If string @c str contains space, wraps the string with double quotes.
    std::string OGRE_MODULE_API AddDoubleQuotesIfNecessary(const std::string &str);

    /// Desanitates asset references in Ogre scripts.
    /** @param script Ogre script data as string.
        @param keywords List of keywords/IDs <b> appended with a space </b>, e.g. "material ", "texture " and "particle_system ". */
    void OGRE_MODULE_API DesanitateAssetIds(std::string &script, const QStringList &keywords);
}

