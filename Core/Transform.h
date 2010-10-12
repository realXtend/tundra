#ifndef incl_Core_CTransform_h
#define incl_Core_CTransform_h

#include "CoreMath.h"
#include "Vector3D.h"

#include <QMetaType>

struct Transform
{
    Vector3df position;
    Vector3df rotation; ///< The rotation of this transform in Euler XYZ.
    Vector3df scale;

    Transform():
        position(0,0,0),
        rotation(0,0,0),
        scale(1,1,1)
    {
    }

    Transform(const Vector3df &pos, const Vector3df &rot, const Vector3df &scale):
        position(pos),
        rotation(rot),
        scale(scale)
    {
    }

    void SetPos(float x, float y, float z)
    {
        position.x = x;
        position.y = y;
        position.z = z;
    }

    void SetRot(float x, float y, float z)
    {
        rotation.x = x;
        rotation.y = y;
        rotation.z = z;
    }

    //! Set scale. Note! scale cannot contain zero value.
    void SetScale(float x, float y, float z)
    {
        scale.x = x;
        scale.y = y;
        scale.z = z;
    }
    
    bool operator == (const Transform& rhs) const
    {
        return (position == rhs.position) && (rotation == rhs.rotation) && (scale == rhs.scale);
    }
    
    bool operator != (const Transform& rhs) const
    {
        return !(*this == rhs);
    }
};

Q_DECLARE_METATYPE(Transform)

#endif
