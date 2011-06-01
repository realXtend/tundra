/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"
#include "Math/Quat.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "Math/float3x3.h"
#include "Math/float3x4.h"
#include "Math/float4x4.h"
#include "Math/MathFunc.h"

Quat::Quat(const float *data)
:x(data[0]),
y(data[1]),
z(data[2]),
w(data[3])
{
}

Quat::Quat(const float3x3 &rotationMatrix)
{
    Set(rotationMatrix);
}

Quat::Quat(const float3x4 &rotationMatrix)
{
    Set(rotationMatrix);
}

Quat::Quat(const float4x4 &rotationMatrix)
{
    Set(rotationMatrix);
}

Quat::Quat(float x_, float y_, float z_, float w_)
:x(x_), y(y_), z(z_), w(w_)
{
}

Quat::Quat(const float3 &rotationAxis, float rotationAngle)
{
    SetFromAxisAngle(rotationAxis, rotationAngle);
}

float3 Quat::WorldX() const
{
    return this->Transform(1.f, 0.f, 0.f);
}

float3 Quat::WorldY() const
{
    return this->Transform(0.f, 1.f, 0.f);
}

float3 Quat::WorldZ() const
{
    return this->Transform(0.f, 0.f, 1.f);
}

float3 Quat::Axis() const
{
    float3 axis;
    float angle;
    ToAxisAngle(axis, angle);
    return axis;
}

float Quat::Angle() const
{
    return acos(w) * 2.f;
}

float Quat::Dot(const Quat &rhs) const
{
    return x*rhs.x + y*rhs.y + z*rhs.z + w*rhs.w;
}

float Quat::LengthSq() const
{
    return x*x + y*y + z*z + w*w;
}

float Quat::Length() const
{
    return sqrtf(LengthSq());
}

float Quat::Normalize()
{
    float length = Length();
    if (length < 1e-4f)
        return 0.f;
    float rcpLength = 1.f / length;
    x *= rcpLength;
    y *= rcpLength;
    z *= rcpLength;
    w *= rcpLength;
    return length;
}

Quat Quat::Normalized() const
{
    Quat copy;
    float success = copy.Normalize();
    assume(success > 0 && "Quat::Normalized failed!");
    return copy;
}

bool Quat::IsNormalized(float epsilon) const
{
    return EqualAbs(LengthSq(), 1.f, epsilon);
}

bool Quat::IsInvertible(float epsilon) const
{
    return LengthSq() > epsilon && IsFinite();
}

bool Quat::IsFinite() const
{
    return isfinite(x) && isfinite(y) && isfinite(z) && isfinite(w);
}

float *Quat::ptr()
{
    return &x;
}

const float *Quat::ptr() const
{
    return &x;
}

void Quat::Inverse()
{
    assume(IsNormalized());
    assume(IsInvertible());
    Conjugate();
}

Quat Quat::Inverted() const
{ 
    assume(IsNormalized());
    assume(IsInvertible());
    return Conjugated();
}

float Quat::InverseAndNormalize()
{
    Conjugate();
    return Normalize();
}

void Quat::Conjugate()
{
    x = -x;
    y = -y;
    z = -z;
}

Quat Quat::Conjugated() const
{
    Quat copy;
    copy.Conjugate();
    return copy;
}

float3 Quat::Transform(const float3 &vec) const
{
    assume(this->IsNormalized());
    float3x3 mat = this->ToFloat3x3();
    return mat * vec;
}

float3 Quat::Transform(float x, float y, float z) const
{
    return Transform(float3(x, y, z));
}

float4 Quat::Transform(const float4 &vec) const
{
    assume(vec.IsWZeroOrOne());

    return float4(Transform(vec.x, vec.y, vec.z), vec.w);
}

Quat Quat::Lerp(const Quat &b, float t) const
{
    assume(0.f <= t && t <= 1.f);
    return *this * (1.f - t) + b * t;
}

Quat Quat::Slerp(const Quat &b, float t) const
{
    assume(0.f <= t && t <= 1.f);
    float angle = acos(Clamp(this->Dot(b), -1.f, 1.f));
    float directionFlip = 1.f;
    if (angle > pi)
    {
        angle = 2.f*pi - angle;
        directionFlip = -1.f;
    }
    if (fabs(angle) < pi/16.f)
        return *this;
    float sina = 1.f / sin(angle);

    return Quat(*this * (sin(angle*(1.f-t))*sina) + b * (sin(angle * t)*sina*directionFlip));
}

Quat Lerp(const Quat &a, const Quat &b, float t)
{
    return a.Lerp(b, t);
}

Quat Slerp(const Quat &a, const Quat &b, float t)
{
    return a.Slerp(b, t);
}

float Quat::AngleBetween(const Quat &target) const
{
    assume(this->IsInvertible());
    Quat q = target / *this;
    return q.Angle();
}

float3 Quat::AxisFromTo(const Quat &target) const
{
    assume(this->IsInvertible());
    Quat q = target / *this;
    return q.Axis();
}

void Quat::ToAxisAngle(float3 &axis, float &angle) const
{
    angle = acos(w) * 2.f;
    float sinz = Sin(angle/2.f);
    if (fabs(sinz) > 1e-4f)
    {
        sinz = 1.f / sinz;
        axis = float3(x * sinz, y * sinz, z * sinz);
    }
    else
    {
        // The quaternion does not produce any rotation. Still, explicitly
        // set the axis so that the user gets a valid normalized vector back.
        angle = 0.f;
        axis = float3(1.f, 0.f, 0.f);
    }
}

void Quat::SetFromAxisAngle(const float3 &axis, float angle)
{
    assume(axis.IsNormalized());
    float cosz = Cos(angle/2.f);
    float sinz = Sin(angle/2.f);
    x = axis.x * sinz;
    y = axis.y * sinz;
    z = axis.z * sinz;
    w = cosz;
}

template<typename M>
void SetQuatFrom(Quat &q, const M &m)
{
    q.w = sqrt(1.f + m[0][0] + m[1][1] + m[2][2]) / 2.f;
    q.x = (m[2][1] - m[1][2]) / (4.f * q.w);
    q.y = (m[0][2] - m[2][0]) / (4.f * q.w);
    q.z = (m[1][0] - m[0][1]) / (4.f * q.w);
    float oldLength = q.Normalize();
    assume(oldLength > 0.f);
}

void Quat::Set(const float3x3 &m)
{
    assume(m.IsOrthogonal());
    assume(m.HasUnitaryScale());
    assume(!m.HasNegativeScale());
    SetQuatFrom(*this, m);
}

void Quat::Set(const float3x4 &m)
{
    assume(m.IsOrthogonal());
    assume(m.HasUnitaryScale());
    assume(!m.HasNegativeScale());
    SetQuatFrom(*this, m);
}

void Quat::Set(const float4x4 &m)
{
    assume(m.IsOrthogonal3());
    assume(m.HasUnitaryScale());
    assume(!m.HasNegativeScale());
    assume(m.Row(3).Equals(0,0,0,1));
    SetQuatFrom(*this, m);
}

void Quat::LookAt(const float3 &localForward, const float3 &targetDirection, const float3 &localUp, const float3 &worldUp)
{
    assume(false && "Not implemented!");
    ///\todo

    assume(localForward.IsNormalized());
    assume(targetDirection.IsNormalized());
    assume(localUp.IsNormalized());
    assume(worldUp.IsNormalized());
}

Quat Quat::RotateX(float angle)
{
    return Quat(float3(1,0,0), angle);
}

Quat Quat::RotateY(float angle)
{
    return Quat(float3(0,1,0), angle);
}

Quat Quat::RotateZ(float angle)
{
    return Quat(float3(0,0,1), angle);
}

Quat Quat::RotateAxisAngle(const float3 &axis, float angle)
{
    return Quat(axis, angle);
}

Quat Quat::RotateFromTo(const float3 &sourceDirection, const float3 &targetDirection)
{
    assume(sourceDirection.IsNormalized());
    assume(targetDirection.IsNormalized());
    float angle = sourceDirection.AngleBetweenNorm(targetDirection);
    assume(angle >= 0.f);
    // If sourceDirection == targetDirection, the cross product comes out zero, and normalization would fail. In that case, pick an arbitrary axis.
    float3 axis = sourceDirection.Cross(targetDirection);
    float oldLength = axis.Normalize();
    if (oldLength == 0)
        axis = float3(1, 0, 0);
    return Quat(axis, angle);
}

Quat Quat::RotateFromTo(const float3 &sourceDirection, const float3 &targetDirection,
    const float3 &sourceDirection2, const float3 &targetDirection2)
{
    assume(sourceDirection.IsNormalized());
    assume(targetDirection.IsNormalized());
    assume(sourceDirection2.IsNormalized());
    assume(targetDirection2.IsNormalized());

    assume(false && "Not implemented!");
    return Quat(); ///\todo
}

Quat Quat::FromEulerXYX(float x2, float y, float x) { return (Quat::RotateX(x2) * Quat::RotateY(y) * Quat::RotateX(x)).Normalized(); }
Quat Quat::FromEulerXZX(float x2, float z, float x) { return (Quat::RotateX(x2) * Quat::RotateZ(z) * Quat::RotateX(x)).Normalized(); }
Quat Quat::FromEulerYXY(float y2, float x, float y) { return (Quat::RotateY(y2) * Quat::RotateX(x) * Quat::RotateY(y)).Normalized(); }
Quat Quat::FromEulerYZY(float y2, float z, float y) { return (Quat::RotateY(y2) * Quat::RotateZ(z) * Quat::RotateY(y)).Normalized(); }
Quat Quat::FromEulerZXZ(float z2, float x, float z) { return (Quat::RotateZ(z2) * Quat::RotateX(x) * Quat::RotateZ(z)).Normalized(); }
Quat Quat::FromEulerZYZ(float z2, float y, float z) { return (Quat::RotateZ(z2) * Quat::RotateY(y) * Quat::RotateZ(z)).Normalized(); }
Quat Quat::FromEulerXYZ(float x, float y, float z) { return (Quat::RotateX(x) * Quat::RotateY(y) * Quat::RotateZ(z)).Normalized(); }
Quat Quat::FromEulerXZY(float x, float z, float y) { return (Quat::RotateX(x) * Quat::RotateZ(z) * Quat::RotateY(y)).Normalized(); }
Quat Quat::FromEulerYXZ(float y, float x, float z) { return (Quat::RotateY(y) * Quat::RotateX(x) * Quat::RotateZ(z)).Normalized(); }
Quat Quat::FromEulerYZX(float y, float z, float x) { return (Quat::RotateY(y) * Quat::RotateZ(z) * Quat::RotateX(x)).Normalized(); }
Quat Quat::FromEulerZXY(float z, float x, float y) { return (Quat::RotateZ(z) * Quat::RotateX(x) * Quat::RotateY(y)).Normalized(); }
Quat Quat::FromEulerZYX(float z, float y, float x) { return (Quat::RotateZ(z) * Quat::RotateY(y) * Quat::RotateX(x)).Normalized(); }

///\todo the following could be heavily optimized. Don't route through float3x3 conversion.

float3 Quat::ToEulerXYX() const { return ToFloat3x3().ToEulerXYX(); }
float3 Quat::ToEulerXZX() const { return ToFloat3x3().ToEulerXZX(); }
float3 Quat::ToEulerYXY() const { return ToFloat3x3().ToEulerYXY(); }
float3 Quat::ToEulerYZY() const { return ToFloat3x3().ToEulerYZY(); }
float3 Quat::ToEulerZXZ() const { return ToFloat3x3().ToEulerZXZ(); }
float3 Quat::ToEulerZYZ() const { return ToFloat3x3().ToEulerZYZ(); }
float3 Quat::ToEulerXYZ() const { return ToFloat3x3().ToEulerXYZ(); }
float3 Quat::ToEulerXZY() const { return ToFloat3x3().ToEulerXZY(); }
float3 Quat::ToEulerYXZ() const { return ToFloat3x3().ToEulerYXZ(); }
float3 Quat::ToEulerYZX() const { return ToFloat3x3().ToEulerYZX(); }
float3 Quat::ToEulerZXY() const { return ToFloat3x3().ToEulerZXY(); }
float3 Quat::ToEulerZYX() const { return ToFloat3x3().ToEulerZYX(); }

float3x3 Quat::ToFloat3x3() const
{
    return float3x3(*this);
}

float3x4 Quat::ToFloat3x4() const
{
    return float3x4(*this);
}

float4x4 Quat::ToFloat4x4() const
{
    return float4x4(*this);
}

std::string Quat::ToString() const
{
    char str[256];
    sprintf(str, "(%.3f, %.3f, %.3f, %.3f)", x, y, z, w);
    return str;
}

std::string Quat::ToString2() const
{
    float3 axis;
    float angle;
    ToAxisAngle(axis, angle);
    char str[256];
    sprintf(str, "Quat(axis:(%.2f,%.2f,%.2f) angle:%2.f)", axis.x, axis.y, axis.z, RadToDeg(angle));
    return str;
}

Quat Quat::operator +(const Quat &rhs) const
{
    return Quat(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

Quat Quat::operator -(const Quat &rhs) const
{
    return Quat(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

Quat Quat::operator *(float scalar) const
{
    return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
}

float3 Quat::operator *(const float3 &rhs) const
{ 
    return Mul(rhs);
}

Quat Quat::operator /(float scalar) const
{
    assume(!EqualAbs(scalar, 0.f));

    return *this * (1.f / scalar);
}

Quat Quat::operator *(const Quat &r) const
{
    return Quat(w*r.w - x*r.x - y*r.y - z*r.z,
                w*r.x + x*r.w + y*r.z - z*r.y,
                w*r.y - x*r.z + y*r.w + z*r.x,
                w*r.z + x*r.y - y*r.x + z*r.w);
}

Quat Quat::operator /(const Quat &rhs) const
{
    Quat inverse = rhs.Inverted();
    return *this * inverse;
}

Quat Quat::Mul(const float3x3 &rhs) const { return *this * Quat(rhs); } 
float3 Quat::Mul(const float3 &vector) const { return this->Transform(vector); }
float4 Quat::Mul(const float4 &vector) const { return this->Transform(vector); }

const Quat Quat::identity = Quat(0.f, 0.f, 0.f, 1.f);
