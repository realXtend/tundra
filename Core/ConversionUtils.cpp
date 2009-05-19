#include "CoreStableHeaders.h"
#include "Quaternion.h"
#include "QuatUtils.h"

namespace Core
{

Core::Vector3df OpenSimToOgreCoordinateAxes(const Core::Vector3df &pos)
{
    return pos;
//    return Core::Vector3df(pos.y, pos.z, pos.x);
}

Core::Quaternion OpenSimToOgreQuaternion(const Core::Quaternion &quat)
{
    return quat;
//    return Core::Quaternion(quat.y, quat.z, quat.x, quat.w);
}

Core::Vector3df OgreToOpenSimCoordinateAxes(const Core::Vector3df &pos)
{
    return pos;
//    return Core::Vector3df(pos.z, pos.x, pos.y);
}

Core::Quaternion OgreToOpenSimQuaternion(const Core::Quaternion &quat)
{
    return quat;
//    return Core::Quaternion(quat.z, quat.x, quat.y, quat.w);
}

}
