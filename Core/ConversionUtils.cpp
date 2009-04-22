#include "CoreStableHeaders.h"
#include "Quaternion.h"
#include "QuatUtils.h"

namespace Core
{

Core::Vector3df OpenSimToOgreCoordinateAxes(const Core::Vector3df &pos)
{
    return Core::Vector3df(pos.y, pos.z, pos.x);
}

Core::Quaternion OpenSimToOgreQuaternion(const Core::Quaternion &quat)
{
    return Core::Quaternion(quat.y, quat.z, quat.x, quat.w);
}

Core::Quaternion OgreToOpenSimQuaternion(const Core::Quaternion &quat)
{
    return Core::Quaternion(quat.x, quat.z, quat.y, quat.w);
}

}
