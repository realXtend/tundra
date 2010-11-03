#ifndef incl_Core_CTransform_h
#define incl_Core_CTransform_h

#include "CoreMath.h"
#include "Vector3D.h"
#include "Matrix4.h"

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
    
    /// Converts this Transform object to a Matrix4 that represents the same Transform. The order of operations is 1. Scale 2. Rotate 3. Translate.
    void ToMatrix4(Matrix4 &dst) const
    {
        ///\todo Our Matrix4 class is bad and should be removed, since it uses D3D (v*M) convention.
        /// This itself is not "bad", but what's bad is that all the rest of our system uses the (M*v) convention.
        Matrix4 translate;
        translate.makeIdentity();
        translate.setTranslation(position);
        
        Matrix4 rotate;
        rotate.setRotationDegrees(rotation);

        Matrix4 scaleMat;
        scaleMat.makeIdentity();
        scaleMat.setScale(scale);

        dst = scaleMat * rotate * translate;        
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
