/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"

#include "Math/MathFunc.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/Triangle.h"
#include "Math/Plane.h"

float3 Triangle::Barycentric(const float3 &point) const
{
    /// @note An alternate mechanism to compute the barycentric is given in Christer Ericson's
    /// Real-Time Collision Detection, pp. 51-52, which might be slightly faster.
    float3 v0 = b - a;
    float3 v1 = c - a;
    float3 v2 = point - a;
    float d00 = Dot(v0, v0);
    float d01 = Dot(v0, v1);
    float d11 = Dot(v1, v1);
    float d20 = Dot(v2, v0);
    float d21 = Dot(v2, v1);
    float denom = 1.f / (d00 * d11 - d01 * d01);
    float v = (d11 * d20 - d01 * d21) * denom;
    float w = (d00 * d21 - d01 * d20) * denom;
    float u = 1.0f - v - w;
    return float3(u, v, w);
}

float3 Triangle::Point(float u, float v, float w) const
{
    return u * a + v * b + w * c;
}

float3 Triangle::Point(const float3 &b) const
{
    return Point(b.x, b.y, b.z);
}

float Triangle::Area() const
{
    return 0.5f * Cross(b-a, c-a).Length();
}

Plane Triangle::GetPlane() const
{
    return Plane(a, b, c);
}

float Triangle::Area2D(const float2 &p1, const float2 &p2, const float2 &p3)
{
    return (p1.x - p2.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p2.y);
}

float Triangle::SignedArea(const float3 &pt, const float3 &a, const float3 &b, const float3 &c)
{
    return Dot(Cross(b-pt, c-pt), Cross(b-a, c-a).Normalized());
}

bool Triangle::IsDegenerate(float epsilon) const
{
    return IsDegenerate(a, b, c);
}

bool Triangle::IsDegenerate(const float3 &a, const float3 &b, const float3 &c, float epsilon)
{
    return a.Equals(b, epsilon) || a.Equals(c, epsilon) || b.Equals(c, epsilon);
}

bool Triangle::Contains(const float3 &point, float triangleThickness) const
{
    if (GetPlane().Distance(point) > triangleThickness)
        return false; ///\todo This test is omitted in Real-Time Collision Detection. p. 25. A bug in the book?

    float3 br = Barycentric(point);
    return br.y >= 0.f && br.z >= 0.f && (br.y + br.z) <= 1.f;
}
