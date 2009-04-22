/// @file ConversionUtils.h
/// @brief Utility functions for converting between coordinate axes and performing (de)serialization.
/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef Core_ConversionUtils_h
#define Core_ConversionUtils_h

#include "Quaternion.h"
#include "Vector3D.h"    

namespace Core
{

/// Transforms a vector from OpenSim coordinate system to the coordinate
/// system we're using in Ogre. OpenSim uses a right-handed system where
/// Z is up, whereas with Ogre we use a RH system where Y is up.
Core::Vector3df OpenSimToOgreCoordinateAxes(const Core::Vector3df &pos);

/// Transforms a quaternion from OpenSim coordinate system to the coordinate
/// system we're using in Ogre. @see Core::Vector3df OpenSimToOgreCoordinateAxes(const Core::Vector3df &pos);
Core::Quaternion OpenSimToOgreQuaternion(const Core::Quaternion &quat);

/// Transforms a quaternion from Ogre coordinate system to the coordinate
/// system we're using in OpenSim.
Core::Quaternion OgreToOpenSimQuaternion(const Core::Quaternion &quat);

}

#endif
