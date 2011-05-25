/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"
#include <cassert>
#include <utility>

#include "Math/float3.h"
#include "Math/float4.h"
#include "Math/float4x4.h"
#include "Math/MathFunc.h"

using namespace std;

float4::float4(float x_, float y_, float z_, float w_)
:x(x_), y(y_), z(z_), w(w_)
{
}

float4::float4(const float3 &xyz, float w_)
:x(xyz.x), y(xyz.y), z(xyz.z), w(w_)
{
}

float4::float4(const float *data)
{
    assert(data);
    x = data[0];
    y = data[1];
    z = data[2];
    w = data[3];
}

float *float4::ptr()
{ 
    return &x;
} 

const float *float4::ptr() const
{ 
    return &x;
} 

const float float4::operator [](int index) const
{ 
    assert(index >= 0);
    assert(index < Size);
    return ptr()[index];
}

float &float4::operator [](int index)
{ 
    assert(index >= 0);
    assert(index < Size);
    return ptr()[index];
}

float3 float4::xyz() const
{
    return float3(x, y, z);
}

float float4::LengthSq3() const
{ 
    return x*x + y*y + z*z;
}

float float4::Length3() const
{ 
    return sqrtf(LengthSq3());
}

float float4::LengthSq4() const
{ 
    return x*x + y*y + z*z + w*w;
}

float float4::Length4() const
{ 
    return sqrtf(LengthSq4());
}

float float4::Normalize3()
{ 
    float lengthSq = LengthSq3();
    if (lengthSq > 1e-6f)
    {
        float length = sqrtf(lengthSq);
        float invLength = 1.f / length;
        x *= invLength;
        y *= invLength;
        z *= invLength;
        return length;
    }
    else
    {
//        printf("float4::Normalize3 called on a vector with 0 length!\n");
        Set(1.f, 0.f, 0.f, w);
        return 0;
    }
}

float4 float4::Normalized3() const
{
    float4 copy = *this;
    float length = copy.Normalize3();
    assume(length > 0);
    return copy;
}

float float4::Normalize4()
{ 
    float lengthSq = LengthSq4();
    if (lengthSq > 1e-6f)
    {
        float length = sqrtf(lengthSq);
        *this *= 1.f / length;
        return length;
    }
    else
    {
        printf("float4::Normalize4 called on a vector with 0 length!\n");
        Set(1.f, 0.f, 0.f, 0.f);
        return false;
    }
}

float4 float4::Normalized4() const
{
    float4 copy = *this;
    float length = copy.Normalize4();
    assume(length > 0);
    return copy;
}

bool float4::NormalizeW()
{
    if (fabs(w) < 1e-6f)
    {
        printf("float4::NormalizeW called on a vector with w=0!\n");
        return false;
    }

    float invW = 1.f / w;
    x *= invW;
    y *= invW;
    z *= invW;
    w = 1.f;
    return true;
}

bool float4::IsWZeroOrOne(float epsilon) const
{
    return EqualAbs(w, 0.f, epsilon) || EqualAbs(w, 1.f, epsilon);
}

bool float4::IsZero4(float epsilonSq) const
{
    return LengthSq4() <= epsilonSq;
}

bool float4::IsZero3(float epsilonSq) const
{
    return LengthSq3() <= epsilonSq;
}

bool float4::IsNormalized4(float epsilonSq) const
{
    return fabs(LengthSq4()-1.f) <= epsilonSq;
}

bool float4::IsNormalized3(float epsilonSq) const
{
    return fabs(LengthSq3()-1.f) <= epsilonSq;
}

void float4::Scale3(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
}

float float4::ScaleToLength3(float newLength)
{
    float length = LengthSq3();
    if (length < 1e-6f)
        return 0.f;

    length = sqrtf(length);
    float scalar = newLength / length;
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return length;
}

bool float4::IsFinite() const
{
    return isfinite(x) && isfinite(y) && isfinite(z) && isfinite(w);
}

bool float4::IsPerpendicular3(const float4 &other, float epsilon) const
{
    return fabs(this->Dot3(other)) < epsilon;
}

std::string float4::ToString() const
{ 
    char str[256];
    sprintf(str, "(%f, %f, %f, %f)", x, y, z, w);
    return std::string(str);
}

float float4::SumOfElements() const
{
    return x + y + z + w;
}

float float4::ProductOfElements() const
{
    return x * y * z * w;
}

float float4::AverageOfElements() const
{
    return (x + y + z + w) / 4.f;
}

float float4::MinElement() const
{
    return min(min(x, y), min(z, w));
}

int float4::MinElementIndex() const
{
    if (x < y) 
    {
        if (z < w)
            return (x < z) ? 0 : 2;
        else
            return (x < w) ? 0 : 3;
    }
    else
    {
        if (z < w)
            return (y < z) ? 1 : 2;
        else
            return (y < w) ? 1 : 3;
    }
}

float float4::MaxElement() const
{
    return max(max(x, y), min(z, w));
}

int float4::MaxElementIndex() const
{
    if (x > y) 
    {
        if (z > w)
            return (x > z) ? 0 : 2;
        else
            return (x > w) ? 0 : 3;
    }
    else
    {
        if (z > w)
            return (y > z) ? 1 : 2;
        else
            return (y > w) ? 1 : 3;
    }
}

float4 float4::Abs() const
{
    return float4(fabs(x), fabs(y), fabs(z), fabs(w));
}

float4 float4::Min(float ceil) const
{
    return float4(min(x, ceil), min(y, ceil), min(z, ceil), min(w, ceil));
}

float4 float4::Min(const float4 &ceil) const
{
    return float4(min(x, ceil.x), min(y, ceil.y), min(z, ceil.z), min(w, ceil.w));
}

float4 float4::Max(float floor) const
{
    return float4(max(x, floor), max(y, floor), max(z, floor), max(w, floor));
}

float4 float4::Max(const float4 &floor) const
{
    return float4(max(x, floor.x), max(y, floor.y), max(z, floor.z), max(w, floor.w));
}

float4 float4::Clamp(const float4 &floor, const float4 &ceil) const
{
    return float4(::Clamp(x, floor.x, ceil.x),
                  ::Clamp(y, floor.y, ceil.y),
                  ::Clamp(z, floor.z, ceil.z),
                  ::Clamp(w, floor.w, ceil.w));
}

float4 float4::Clamp01() const
{
    return float4(::Clamp(x, 0.f, 1.f),
                  ::Clamp(y, 0.f, 1.f),
                  ::Clamp(z, 0.f, 1.f),
                  ::Clamp(w, 0.f, 1.f));
}

float4 float4::Clamp(float floor, float ceil) const
{
    return float4(::Clamp(x, floor, ceil),
                  ::Clamp(y, floor, ceil),
                  ::Clamp(z, floor, ceil),
                  ::Clamp(w, floor, ceil));
}

float float4::Distance3Sq(const float4 &rhs) const
{
    float dx = x - rhs.x;
    float dy = y - rhs.y;
    float dz = z - rhs.z;
    return dx*dx + dy*dy + dz*dz;
}

float float4::Distance3(const float4 &rhs) const
{
    return sqrtf(Distance3Sq(rhs));
}

float float4::Dot3(const float3 &rhs) const
{
    return x * rhs.x + y * rhs.y + z * rhs.z;
}

float float4::Dot3(const float4 &rhs) const
{
    return x * rhs.x + y * rhs.y + z * rhs.z;
}

float float4::Dot4(const float4 &rhs) const
{
    return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
}

/** dst = A x B - The standard cross product:
\code
        |a cross b| = |a||b|sin(alpha)
    
        i        j        k        i        j        k        units (correspond to x,y,z)
        a        b        c        a        b        c        this vector
        d        e        f        d        e        f        vector v
        -cei    -afj    -bdk    bfi    cdj    aek    result
    
        x = bfi - cei = (bf-ce)i;
        y = cdj - afj = (cd-af)j;
        z - aek - bdk = (ae-bd)k;
\endcode

Cross product is anti-commutative, i.e. a x b == -b x a.
It distributes over addition, meaning that a x (b + c) == a x b + a x c,
and combines with scalar multiplication: (sa) x b == a x (sb). 
i x j == -(j x i) == k,
(j x k) == -(k x j) == i,
(k x i) == -(i x k) == j. */
float4 float4::Cross3(const float3 &rhs) const
{
    float4 dst;
    dst.x = y * rhs.z - z * rhs.y;
    dst.y = z * rhs.x - x * rhs.z;
    dst.z = x * rhs.y - y * rhs.x;
    dst.w = 0.f;
    return dst;
}

float4 float4::Cross3(const float4 &rhs) const
{
    return Cross3(rhs.xyz());
}

float4x4 float4::OuterProduct(const float4 &rhs) const
{
    const float4 &u = *this;
    const float4 &v = rhs;
    return float4x4(u[0]*v[0], u[0]*v[1], u[0]*v[2], u[0]*v[3],
                    u[1]*v[0], u[1]*v[1], u[1]*v[2], u[1]*v[3],
                    u[2]*v[0], u[2]*v[1], u[2]*v[2], u[2]*v[3],
                    u[3]*v[0], u[3]*v[1], u[3]*v[2], u[3]*v[3]);
}

float4 float4::Perpendicular3(const float3 &hint, const float3 &hint2) const
{
    assume(!this->IsZero3());
    assume(EqualAbs(w, 0));
    assume(hint.IsNormalized());
    assume(hint2.IsNormalized());
    float3 v = this->Cross3(hint).xyz();
    float len = v.Normalize();
    if (len == 0)
        return float4(hint2, 0);
    else
        return float4(v, 0);
}

float4 float4::AnotherPerpendicular3(const float3 &hint, const float3 &hint2) const
{
    float4 firstPerpendicular = Perpendicular3(hint, hint2);
    float4 v = this->Cross3(firstPerpendicular);
    return v.Normalized3();
}

float4 float4::Reflect3(const float3 &normal) const
{
    assume(normal.IsNormalized());
    assume(EqualAbs(w, 0));
    return 2.f * this->ProjectToNorm3(normal) - *this;
}

float float4::AngleBetween3(const float4 &other) const
{
    return acos(Dot3(other)) / sqrt(LengthSq3() * other.LengthSq3());
}

float float4::AngleBetweenNorm3(const float4 &other) const
{
    assert(this->IsNormalized3());
    assert(other.IsNormalized3());
    return acos(Dot3(other));
}

float float4::AngleBetween4(const float4 &other) const
{
    return acos(Dot4(other)) / sqrt(LengthSq3() * other.LengthSq3());
}

float float4::AngleBetweenNorm4(const float4 &other) const
{
    assert(this->IsNormalized4());
    assert(other.IsNormalized4());
    return acos(Dot4(other));
}

float4 float4::ProjectTo3(const float3 &target) const
{
    assume(!target.IsZero());
    assume(this->IsWZeroOrOne());
    return float4(target * Dot(xyz(), target) / target.LengthSq(), w);
}

float4 float4::ProjectToNorm3(const float3 &target) const
{
    assume(target.IsNormalized());
    assume(this->IsWZeroOrOne());
    return float4(target * Dot(xyz(), target), w);
}

float4 float4::FromScalar(float scalar)
{ 
    return float4(scalar, scalar, scalar, scalar);
}

float4 float4::FromScalar(float scalar, float w)
{ 
    return float4(scalar, scalar, scalar, w);
}

void float4::SetFromScalar(float scalar)
{
    x = scalar;
    y = scalar;
    z = scalar;
    w = scalar;
}

void float4::Set(float x_, float y_, float z_, float w_)
{
    x = x_;
    y = y_;
    z = z_;
    w = w_;
}

void float4::SetFromScalar(float scalar, float w_)
{
    x = scalar;
    y = scalar;
    z = scalar;
    w = w_;
}

bool float4::Equals(const float4 &other, float epsilon) const
{
    return fabs(x - other.x) < epsilon &&
           fabs(y - other.y) < epsilon &&
           fabs(z - other.z) < epsilon &&
           fabs(w - other.w) < epsilon;
}

bool float4::Equals(float x_, float y_, float z_, float w_, float epsilon) const
{
    return fabs(x - x_) < epsilon &&
           fabs(y - y_) < epsilon &&
           fabs(z - z_) < epsilon &&
           fabs(w - w_) < epsilon;
}

float4 float4::operator +(const float4 &rhs) const
{
    return float4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

float4 float4::operator -(const float4 &rhs) const
{
    return float4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

float4 float4::operator -() const
{
    return float4(-x, -y, -z, -w);
}
/*
float4 float4::operator *(const float4 &rhs) const
{
    return float4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
}
*/
float4 float4::operator *(float scalar) const
{
    return float4(x * scalar, y * scalar, z * scalar, w * scalar);
}

float4 operator *(float scalar, const float4 &rhs)
{
    return float4(scalar * rhs.x, scalar * rhs.y, scalar * rhs.z, scalar * rhs.w);
}
/*
float4 float4::operator /(const float4 &rhs) const
{
    return float4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
}
*/
float4 float4::operator /(float scalar) const
{
    float invScalar = 1.f / scalar;
    return float4(x * invScalar, y * invScalar, z * invScalar, w * invScalar);
}
/*
float4 operator /(float scalar, const float4 &rhs)
{
    return float4(scalar / rhs.x, scalar / rhs.y, scalar / rhs.z, scalar / rhs.w);
}
*/
float4 &float4::operator +=(const float4 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;

    return *this;
}

float4 &float4::operator -=(const float4 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;

    return *this;
}
/*
float4 &float4::operator *=(const float4 &rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    w *= rhs.w;

    return *this;
}
*/
float4 &float4::operator *=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;

    return *this;
}
/*
float4 &float4::operator /=(const float4 &rhs)
{
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    w /= rhs.w;

    return *this;
}
*/
float4 &float4::operator /=(float scalar)
{
    float invScalar = 1.f / scalar;
    x *= invScalar;
    y *= invScalar;
    z *= invScalar;
    w *= invScalar;

    return *this;
}

std::ostream &operator <<(std::ostream &out, const float4 &rhs)
{
    std::string str = rhs.ToString();
    out << str;
    return out;
}

const float4 float4::zero = float4(0, 0, 0, 0);
const float4 float4::one = float4(1, 1, 1, 1);
const float4 float4::unitX = float4(1, 0, 0, 0);
const float4 float4::unitY = float4(0, 1, 0, 0);
const float4 float4::unitZ = float4(0, 0, 1, 0);
const float4 float4::unitW = float4(0, 0, 0, 1);
