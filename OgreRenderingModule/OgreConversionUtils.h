// For conditions of distribution and use, see copyright notice in license.txt
/// @file OgreConversionUtils.h
/// Contains some common methods for conversions between Ogre and Core variable types.

#ifndef incl_OgreConversionUtils_h
#define incl_OgreConversionUtils_h

#include <OgreColourValue.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

#include "Color.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "OgreModuleApi.h"

namespace OgreRenderer
{
    /// Converts Color to Ogre::ColourValue.
    Ogre::ColourValue OGRE_MODULE_API ToOgreColor(const Color &color);
    
    /// Converts Ogre::ColourValue to Color.
    Color OGRE_MODULE_API ToCoreColor(const Ogre::ColourValue &ogreColour);

    /// Converts Vector3f to Ogre::Vector3.
    Ogre::Vector3 OGRE_MODULE_API ToOgreVector3(const Vector3df &vector);

    /// Converts Quaternion to Ogre::Quaternion
    Ogre::Quaternion OGRE_MODULE_API ToOgreQuaternion(const Quaternion &quat);
}

#endif
