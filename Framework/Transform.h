#ifndef incl_Framework_CTransform_h
#define incl_Framework_CTransform_h

#include "CoreMath.h"
#include "Vector3D.h"

#include <QMetaType>
#include "Math/float4.h"
#include "Math/float3x4.h"
#include "Math/float4x4.h"
#include "Math/TransformOps.h"
#include "Math/Quat.h"
#include "Math/MathFunc.h"

class Transform
{
public:
    float3 pos;
    /// Specifies the rotation of this transform in *degrees*, using the Euler ZYX convention.
    /// This means that vertex v is processed using the formula Rz * Ry * Rx * v, where
    /// Rz, Ry and Rx are elementary counter-clockwise rotations about the cardinal axes z, y and x.
    /// rotation.x stores the rotation angle (in degrees) of the matrix Rx,
    /// rotation.y stores the rotation angle (in degrees) of the matrix Ry, and
    /// rotation.z stores the rotation angle (in degrees) of the matrix Rz.
    float3 rot; 
    float3 scale;

    Transform()
    :pos(0,0,0),
    rot(0,0,0),
    scale(1,1,1)
    {
    }

    Transform(const Vector3df &pos_, const Vector3df &rot_, const Vector3df &scale)
    :pos(pos_),
    rot(rot_),
    scale(scale)
    {
    }

    Transform(const float3x3 &m)
    :pos(0,0,0),
    scale(1,1,1)
    {
        SetRotation(m);
    }

    Transform(const float3x4 &m)
    {
        FromFloat3x4(m);
    }

    Transform(const float4x4 &m)
    {
        FromFloat4x4(m);
    }

    void SetPos(float x, float y, float z)
    {
        pos.x = x;
        pos.y = y;
        pos.z = z;
    }

    void SetRot(float x, float y, float z)
    {
        rot.x = x;
        rot.y = y;
        rot.z = z;
    }

    /// Set scale. Note! scale cannot contain zero value.
    void SetScale(float x, float y, float z)
    {
        scale.x = x;
        scale.y = y;
        scale.z = z;
    }

    float3x4 ToFloat3x4() const
    {
        return float3x4::Translate(pos) * float3x4::FromEulerZYX(DegToRad(rot.z), DegToRad(rot.y), DegToRad(rot.x)) * float3x4::Scale(scale);
    }

    float4x4 ToFloat4x4() const
    {
        return float4x4::Translate(pos) * float4x4::FromEulerZYX(DegToRad(rot.z), DegToRad(rot.y), DegToRad(rot.x)) * float4x4::Scale(scale);
    }

    void FromFloat3x4(const float3x4 &m)
    {
        float3 trans;
        Quat rot_;
        float3 scl;
        m.Decompose(trans, rot_, scl);
        pos = trans;
        rot = RadToDeg(rot_.ToEulerZYX());
        std::swap(rot.x, rot.z); // The above function returns a vector with convention [0] [1] [2]: Z Y X, whereas we want to store it in [0] [1] [2] : X Y Z.
        scale = scl;
    }

    void FromFloat4x4(const float4x4 &m)
    {
        assume(m.Row(3).Equals(0,0,0,1));
        float3 trans;
        Quat rot_;
        float3 scl;
        m.Decompose(trans, rot_, scl);
        pos = trans;
        rot = RadToDeg(rot_.ToEulerXYZ());
        std::swap(rot.x, rot.z); // The above function returns a vector with convention [0] [1] [2]: Z Y X, whereas we want to store it in [0] [1] [2] : X Y Z.
        scale = scl;
    }

    void SetRotation(const float3x3 &mat)
    {
        rot = RadToDeg(mat.ToEulerXYZ());
        std::swap(rot.x, rot.z); // The above function returns a vector with convention [0] [1] [2]: Z Y X, whereas we want to store it in [0] [1] [2] : X Y Z.
    }

    void SetRotation(const Quat &q)
    {
        rot = RadToDeg(q.ToEulerXYZ());
        std::swap(rot.x, rot.z); // The above function returns a vector with convention [0] [1] [2]: Z Y X, whereas we want to store it in [0] [1] [2] : X Y Z.
    }

    /// Returns the rotation part of this Transform as a float3x3.
    float3x3 Rotation3x3() const
    {
        return float3x3::FromEulerZYX(DegToRad(rot.z), DegToRad(rot.y), DegToRad(rot.x));
    }

    /// Returns the rotation part of this Transform as a quaternion.
    Quat RotationQuat() const
    {
        return Quat::FromEulerZYX(DegToRad(rot.z), DegToRad(rot.y), DegToRad(rot.x));
    }
    
    Transform operator *(const Transform &rhs) const
    {
        Transform res;
        float3x4 m = this->ToFloat3x4() * rhs.ToFloat3x4();
        res.FromFloat3x4(m);
        return res;
    }

    Transform Mul(const Transform &rhs) const
    {
        return *this * rhs;
    }

    operator float3x4() const
    {
        return ToFloat3x4();
    }

    operator float4x4() const
    {
        return ToFloat4x4();
    }

    bool operator ==(const Transform &rhs) const
    {
        return pos == rhs.pos && rot == rhs.rot && scale == rhs.scale;
    }
    
    bool operator !=(const Transform &rhs) const
    {
        return !(*this == rhs);
    }

    QString toString() const
    {
        return "Transform(Pos:(" + QString::number(pos.x) + "," + QString::number(pos.y) + "," + QString::number(pos.z) + ") Rot:(" +
            QString::number(rot.x) + "," + QString::number(rot.y) + "," + QString::number(rot.z) + " Scale:(" +
            QString::number(scale.x) + "," + QString::number(scale.y) + "," + QString::number(scale.z) + "))";
    }

};

Q_DECLARE_METATYPE(Transform)
Q_DECLARE_METATYPE(Transform*)

#endif
