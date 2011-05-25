/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"
#include <cassert>
#include <utility>

#include "Math/float2.h"
#include "Math/MathFunc.h"

using namespace std;

float2::float2(float x_, float y_)
:x(x_), y(y_)
{
}

float2::float2(const float *data)
{
    assert(data);
    x = data[0];
    y = data[1];
}

float *float2::ptr()
{ 
    return &x;
} 

const float *float2::ptr() const
{ 
    return &x;
} 

const float float2::operator [](int index) const
{ 
    assert(index >= 0);
    assert(index < Size);
    return ptr()[index];
}

float &float2::operator [](int index)
{ 
    assert(index >= 0);
    assert(index < Size);
    return ptr()[index];
}

float float2::LengthSq() const
{ 
    return x*x + y*y;
}

float float2::Length() const
{ 
    return sqrtf(LengthSq());
}

float float2::Normalize()
{ 
    float lengthSq = LengthSq();
    if (lengthSq > 1e-6f)
    {
        float length = sqrtf(lengthSq);
        *this *= 1.f / length;
        return length;
    }
    else
    {
        printf("float2::Normalize called on a vector with 0 length!\n");
        Set(1.f, 0.f);
        return 0;
    }
}

float2 float2::Normalized() const
{
    float2 copy = *this;
    float oldLength = copy.Normalize();
    assume(oldLength > 0.f && "float2::Normalized() failed!");
    return copy;
}

float float2::ScaleToLength(float newLength)
{
    float length = LengthSq();
    if (length < 1e-6f)
        return 0.f;

    length = sqrtf(length);
    float scalar = newLength / length;
    x *= scalar;
    y *= scalar;
    return length;
}

bool float2::IsNormalized(float epsilonSq) const
{
    return fabs(LengthSq()-1.f) <= epsilonSq;
}

bool float2::IsZero(float epsilonSq) const
{
    return fabs(LengthSq()) <= epsilonSq;
}

bool float2::IsFinite() const
{
    return isfinite(x) && isfinite(y);
}

bool float2::IsPerpendicular(const float2 &other, float epsilon) const
{
    return fabs(Dot(other)) <= epsilon;
}

std::string float2::ToString() const
{ 
    char str[256];
    sprintf(str, "(%f, %f)", x, y);
    return std::string(str);
}

float float2::SumOfElements() const
{
    return x + y;
}

float float2::ProductOfElements() const
{
    return x * y;
}

float float2::AverageOfElements() const
{
    return (x + y) / 2.f;
}

float float2::MinElement() const
{
    return min(x, y);
}

int float2::MinElementIndex() const
{
    return (x <= y) ? 0 : 1;
}

float float2::MaxElement() const
{
    return max(x, y);
}

int float2::MaxElementIndex() const
{
    return (x > y) ? 0 : 1;
}

float2 float2::Abs() const
{
    return float2(fabs(x), fabs(y));
}

float2 float2::Min(float floor) const
{
    return float2(min(x, floor), min(x, floor));
}

float2 float2::Min(const float2 &floor) const
{
    return float2(min(x, floor.x), min(x, floor.x));
}

float2 float2::Max(float ceil) const
{
    return float2(max(x, ceil), max(x, ceil));
}

float2 float2::Max(const float2 &ceil) const
{
    return float2(max(x, ceil.x), max(x, ceil.x));
}

float2 float2::Clamp(const float2 &floor, const float2 &ceil) const
{
    return float2(::Clamp(x, floor.x, ceil.x), ::Clamp(y, floor.y, ceil.y));
}

float2 float2::Clamp(float floor, float ceil) const
{
    return float2(::Clamp(x, floor, ceil), ::Clamp(y, floor, ceil));
}

float2 float2::Clamp01() const
{
    return Clamp(0.f, 1.f);
}

float float2::DistanceSq(const float2 &rhs) const
{
    float dx = x - rhs.x;
    float dy = y - rhs.y;
    return dx*dx + dy*dy;
}

float float2::Distance(const float2 &rhs) const
{
    return sqrtf(DistanceSq(rhs));
}

float float2::Dot(const float2 &rhs) const
{
    return x * rhs.x + y * rhs.y;
}

float float2::PerpDot(const float2 &rhs) const
{
    return -y * rhs.x + x * rhs.y;
}

float2 float2::Reflect(const float2 &normal) const
{
    assume(normal.IsNormalized());
    return 2.f * this->ProjectToNorm(normal) - *this;
}

float2 float2::Refract(const float2 &normal, float negativeSideRefractionIndex, float positiveSideRefractionIndex) const
{
    assume(false && "Not implemented!"); ///\todo
    return float2(0.f,0.f);
}

float2 float2::ProjectTo(const float2 &direction) const
{
    assume(!direction.IsZero());
    return direction * this->Dot(direction) / direction.LengthSq();
}

float2 float2::ProjectToNorm(const float2 &direction) const
{
    assume(direction.IsNormalized());
    return direction * this->Dot(direction);
}

float float2::AngleBetween(const float2 &other) const
{
    return acos(Dot(other)) / sqrt(LengthSq() * other.LengthSq());
}

float float2::AngleBetweenNorm(const float2 &other) const
{
    assert(this->IsNormalized());
    assert(other.IsNormalized());
    return acos(Dot(other));
}

void float2::Decompose(const float2 &direction, float2 &outParallel, float2 &outPerpendicular) const
{
    assume(false && "Not implemented!"); ///\todo
}

void float2::Orthogonalize(const float2 &a, float2 &b)
{
    assume(false && "Not implemented!"); ///\todo
}

void float2::Orthonormalize(float2 &a, float2 &b)
{
    assume(false && "Not implemented!"); ///\todo
}

float2 float2::FromScalar(float scalar)
{ 
    return float2(scalar, scalar);
}

void float2::SetFromScalar(float scalar)
{
    x = scalar;
    y = scalar;
}

void float2::Set(float x_, float y_)
{
    x = x_;
    y = y_;
}

void float2::Rotate90CW()
{
    float oldX = x;
    x = y;
    y = -oldX;
}
    
void float2::Rotate90CCW()
{
    float oldX = x;
    x = -y;
    y = oldX;
}

bool float2::OrientedCCW(const float2 &a, const float2 &b, const float2 &c)
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

float2 float2::operator +(const float2 &rhs) const
{
    return float2(x + rhs.x, y + rhs.y);
}

float2 float2::operator -(const float2 &rhs) const
{
    return float2(x - rhs.x, y - rhs.y);
}

float2 float2::operator -() const
{
    return float2(-x, -y);
}

/*
float2 float2::operator *(const float2 &rhs) const
{
    return float2(x * rhs.x, y * rhs.y);
}
*/
float2 float2::operator *(float scalar) const
{
    return float2(x * scalar, y * scalar);
}

float2 operator *(float scalar, const float2 &rhs)
{
    return float2(scalar * rhs.x, scalar * rhs.y);
}
/*
float2 float2::operator /(const float2 &rhs) const
{
    return float2(x / rhs.x, y / rhs.y);
}
*/
float2 float2::operator /(float scalar) const
{
    float invScalar = 1.f / scalar;
    return float2(x * invScalar, y * invScalar);
}

float2 operator /(float scalar, const float2 &rhs)
{
    return float2(scalar / rhs.x, scalar / rhs.y);
}

float2 &float2::operator +=(const float2 &rhs)
{
    x += rhs.x;
    y += rhs.y;

    return *this;
}

float2 &float2::operator -=(const float2 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;

    return *this;
}
/*
float2 &float2::operator *=(const float2 &rhs)
{
    x *= rhs.x;
    y *= rhs.y;

    return *this;
}
*/
float2 &float2::operator *=(float scalar)
{
    x *= scalar;
    y *= scalar;

    return *this;
}
/*
float2 &float2::operator /=(const float2 &rhs)
{
    x /= rhs.x;
    y /= rhs.y;

    return *this;
}
*/
float2 &float2::operator /=(float scalar)
{
    float invScalar = 1.f / scalar;
    x *= invScalar;
    y *= invScalar;

    return *this;
}

std::ostream &operator <<(std::ostream &out, const float2 &rhs)
{
    std::string str = rhs.ToString();
    out << str;
    return out;
}

const float2 float2::zero = float2(0, 0);
const float2 float2::one = float2(1, 1);
const float2 float2::unitX = float2(1, 0);
const float2 float2::unitY = float2(0, 1);
