#include "CoreStableHeaders.h"
#include "Quaternion.h"
#include "QuatUtils.h"

Vector3df OpenSimToOgreCoordinateAxes(const Vector3df &pos)
{
    return pos;
//    return Vector3df(pos.y, pos.z, pos.x);
}

Quaternion OpenSimToOgreQuaternion(const Quaternion &quat)
{
    return quat;
//    return Quaternion(quat.y, quat.z, quat.x, quat.w);
}

Vector3df OgreToOpenSimCoordinateAxes(const Vector3df &pos)
{
    return pos;
//    return Vector3df(pos.z, pos.x, pos.y);
}

Quaternion OgreToOpenSimQuaternion(const Quaternion &quat)
{
    return quat;
//    return Quaternion(quat.z, quat.x, quat.y, quat.w);
}
