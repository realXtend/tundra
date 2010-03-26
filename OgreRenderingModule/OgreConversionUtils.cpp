// For conditions of distribution and use, see copyright notice in license.txt
/// @file OgreConversionUtils.cpp
/// Contains some common methods for conversions between Ogre and Core variable types.

#include "StableHeaders.h"

#include "OgreConversionUtils.h"

namespace OgreRenderer
{

Ogre::ColourValue ToOgreColor(const Color& color)
{
	return Ogre::ColourValue(color.r, color.g, color.b, color.a);
}

Color ToCoreColor(const Ogre::ColourValue& ogreColour)
{
   return Color(ogreColour.r, ogreColour.g, ogreColour.b, ogreColour.a);
}

Ogre::Vector3 ToOgreVector3(const Vector3df &vector)
{
    return Ogre::Vector3(vector.x, vector.y, vector.z);
}

Ogre::Quaternion ToOgreQuaternion(const Quaternion &quat)
{
    return Ogre::Quaternion(quat.w, quat.x, quat.y, quat.z);
}

}
