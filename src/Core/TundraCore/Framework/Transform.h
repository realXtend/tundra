/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   Transform.h
    @brief  Describes transformation of an object in 3D space. */

#pragma once

#include "TundraCoreApi.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "Math/float3x4.h"
#include "Math/float4x4.h"
#include "Math/TransformOps.h"
#include "Math/Quat.h"
#include "Math/MathFunc.h"

#include <QMetaType>

/// Describes transformation of an object in 3D space.
/** Can be used as entity-component attribute. */
class TUNDRACORE_API Transform
{
public:
    float3 pos;
    /// Specifies the rotation of this transform in *degrees*, using the Euler ZYX convention.
    /// This means that vertex v is processed using the formula Rz * Ry * Rx * v, where
    /// Rz, Ry and Rx are elementary counter-clockwise rotations about the cardinal axes z, y and x.
    /// NOTE: The angles are stored in an unconventional order:
    /// rot.x stores the rotation angle (in degrees) of the matrix Rx,
    /// rot.y stores the rotation angle (in degrees) of the matrix Ry, and
    /// rot.z stores the rotation angle (in degrees) of the matrix Rz.
    /// (Instead of having rot[0] store the rotation about Rz, rot[1] about Ry, rot[2] about Rx, as
    ///  the math libraries do).
    float3 rot;
    float3 scale; ///< @note scale cannot contain zero value.

    /// The default ctor initializes pos to float3::zero, rot to float3::zero, and scale to float3::one
    Transform()
    :pos(0,0,0),
    rot(0,0,0),
    scale(1,1,1)
    {
    }

    Transform(const float3 &pos_, const float3 &rot_, const float3 &scale_)
    :pos(pos_),
    rot(rot_),
    scale(scale_)
    {
    }

    explicit Transform(const float3x3 &m)
    :pos(0,0,0),
    scale(1,1,1)
    {
        SetOrientation(m);
    }

    explicit Transform(const float3x4 &m)
    {
        FromFloat3x4(m);
    }

    explicit Transform(const float4x4 &m)
    {
        FromFloat4x4(m);
    }

    void SetPos(const float3 &v)
    {
        assume(v.IsFinite());
        pos.x = v.x;
        pos.y = v.y;
        pos.z = v.z;
    }

    void SetPos(float x, float y, float z)
    {
        assume(isfinite(x) && isfinite(y) && isfinite(z));
        pos.x = x;
        pos.y = y;
        pos.z = z;
    }

    /// Direcly sets the rotation angles in euler ZYX convention, in degrees.
    void SetRotation(float x, float y, float z)
    {
        assume(isfinite(x) && isfinite(y) && isfinite(z));
        rot.x = x;
        rot.y = y;
        rot.z = z;
    }

    /// Set scale. Note! scale cannot contain zero value.
    void SetScale(float x, float y, float z)
    {
        assume(isfinite(x) && isfinite(y) && isfinite(z));
        scale.x = x;
        scale.y = y;
        scale.z = z;
    }

    void SetScale(const float3 &s)
    {
        assume(s.IsFinite());
        scale = s;
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
        assume(m.IsFinite());
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
        assume(m.IsFinite());
        assume(m.Row(3).Equals(0,0,0,1));
        float3 trans;
        Quat rot_;
        float3 scl;
        m.Decompose(trans, rot_, scl);
        pos = trans;
        rot = RadToDeg(rot_.ToEulerZYX());
        std::swap(rot.x, rot.z); // The above function returns a vector with convention [0] [1] [2]: Z Y X, whereas we want to store it in [0] [1] [2] : X Y Z.
        scale = scl;
    }

    void SetRotationAndScale(const float3x3 &mat)
    {
        assume(mat.IsFinite());
        scale.x = mat.Col(0).Length();
        scale.y = mat.Col(1).Length();
        scale.z = mat.Col(2).Length();
        float3x3 m = mat;
        m.RemoveScale();
        SetOrientation(m);
    }

    /// Sets the rotation part of this transform.
    void SetOrientation(const float3x3 &mat)
    {
        assume(mat.IsFinite());
        rot = RadToDeg(mat.ToEulerZYX());
        std::swap(rot.x, rot.z); // The above function returns a vector with convention [0] [1] [2]: Z Y X, whereas we want to store it in [0] [1] [2] : X Y Z.
    }

    /// Sets the rotation part of this transform.
    void SetOrientation(const Quat &q)
    {
        assume(q.IsFinite());
        rot = RadToDeg(q.ToEulerZYX());
        std::swap(rot.x, rot.z); // The above function returns a vector with convention [0] [1] [2]: Z Y X, whereas we want to store it in [0] [1] [2] : X Y Z.
    }

    /// Returns the rotation part of this Transform as a float3x3.
    float3x3 Orientation3x3() const
    {
        return float3x3::FromEulerZYX(DegToRad(rot.z), DegToRad(rot.y), DegToRad(rot.x));
    }

    /// Returns the rotation part of this Transform as a quaternion.
    Quat Orientation() const
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
        return pos.Equals(rhs.pos) && rot.Equals(rhs.rot) && scale.Equals(rhs.scale);
    }
    
    bool operator !=(const Transform &rhs) const
    {
        return !(*this == rhs);
    }

    /// Implicit conversion to string.
    /** @see ToString */
    operator QString() const;

    /// Returns "Transform(Pos:(x,y,z)) Rot:(x,y,z) Scale:(x,y,z))".
    QString ToString() const { return (QString)*this; }

    /// For QtScript-compatibility.
    QString toString() const { return (QString)*this; }

    /// Returns "px,py,pz,rx,ry,rz,sx,sy,sz".
    /** This is the preferred format for the Transfrom if it has to be serialized to a string for machine transfer.
        @todo Remove commas from the string. */
    QString SerializeToString() const;

    /// Parses a string to a new Color.
    /** Accepted format is "px,py,pz,rx,ry,rz,sx,sy,sz".
        @sa SerializeToString */
    static Transform FromString(const char *str);
    static Transform FromString(const QString &str) { return FromString(str.simplified().toStdString().c_str()); } ///< @overload
};

Q_DECLARE_METATYPE(Transform)
Q_DECLARE_METATYPE(Transform*)
