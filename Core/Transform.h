#ifndef incl_Core_CTransform_h
#define incl_Core_CTransform_h

#include "CoreMath.h"
#include "Vector3D.h"

struct Transform
{
    Vector3D<float> position;
    Vector3D<float> rotation;
    Vector3D<float> scale;

    Transform():
        position(0,0,0),
        rotation(0,0,0),
        scale(1,1,1)
    {
    }

    Transform(const Vector3D<float> &pos, const Vector3D<float> &rot, const Vector3D<float> &scale):
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

        // Ogre doesn't like if we are setting scale value to zero so we add very small number instead.
        if (fabs(x) < 1e-4f)
            scale.x = 1e-4f;
        if (fabs(y) < 1e-4f)
            scale.y = 1e-4f;
        if (fabs(z) < 1e-4f)
            scale.z = 1e-4f;
    }
};
#endif