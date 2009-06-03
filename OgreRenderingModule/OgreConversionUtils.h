// For conditions of distribution and use, see copyright notice in license.txt
/// @file OgreConversionUtils.h
/// Contains some common methods for conversions between Ogre and Core variable types.

#ifndef incl_OgreConversionUtils_h
#define incl_OgreConversionUtils_h

#include <Ogre.h>

#include "OgreModuleApi.h"

namespace OgreRenderer
{
    /// Converts Core::Color to Ogre::ColourValue.
    Ogre::ColourValue OGRE_MODULE_API ToOgreColor(const Core::Color &color);
    
    /// Converts Ogre::ColourValue to Core::Color.
    Core::Color OGRE_MODULE_API ToCoreColor(const Ogre::ColourValue &ogreColour);

    /// Converts Core::Vector3f to Ogre::Vector3.
    Ogre::Vector3 OGRE_MODULE_API ToOgreVector3(const Core::Vector3df &vector);

    /// Converts Core::Quaternion to Ogre::Quaternion
    Ogre::Quaternion OGRE_MODULE_API ToOgreQuaternion(const Core::Quaternion &quat);
}

#endif
