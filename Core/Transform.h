#ifndef incl_Core_CTransform_h
#define incl_Core_CTransform_h

#include "CoreMath.h"
#include "Vector3D.h"

struct Transform
{
    Vector3D<Real> position;
    Vector3D<Real> rotation;
    Vector3D<Real> scale;

    Transform():
        position(0),
        rotation(0),
        scale(1)
    {
    }

    Transform(const Transform &other):
        position(other.position),
        rotation(other.rotation),
        scale(other.scale)
    {
    }

    Transform(const Vector3D<Real> &pos, const Vector3D<Real> &rot, const Vector3D<Real> &scale):
        position(pos),
        rotation(rot),
        scale(scale)
    {
    }

    void SetPos(Real x, Real y, Real z)
    {
        position.x = x;
        position.y = y;
        position.z = z;
    }

    void SetRot(Real x, Real y, Real z)
    {
        rotation.x = x;
        rotation.y = y;
        rotation.z = z;
    }

    //! Set scale. Note! scale cannot contain zero value.
    void SetScale(Real x, Real y, Real z)
    {
        scale.x = x;
        scale.y = y;
        scale.z = z;
        // Ogre dont like if we are setting scale value to zero so we add very small number instead.
        if(x == 0)
            scale.x += 0.0000001f;
        if(y == 0)
            scale.y += 0.0000001f;
        if(z == 0)
            scale.z += 0.0000001f;
    }
};
#endif