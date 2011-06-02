/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"
#include "Line.h"
#include "Ray.h"
#include "LineSegment.h"
#include "float3x3.h"
#include "float3x4.h"
#include "float4x4.h"
#include "Quat.h"
#include "MathFunc.h"

Ray::Ray(const float3 &pos_, const float3 &dir_)
:pos(pos_), dir(dir_)
{
    assume(dir.IsNormalized());
}

Ray::Ray(const Line &line)
:pos(line.pos), dir(line.dir)
{
    assume(dir.IsNormalized());
}

Ray::Ray(const LineSegment &lineSegment)
:pos(lineSegment.a), dir(lineSegment.Dir())
{
}

/// Returns a point on this line.
float3 Ray::GetPoint(float d) const
{
    assert(dir.IsNormalized());
    return pos + d * dir;
}

/// Applies a transformation to this line.
void Ray::Transform(const float3x3 &transform)
{
    pos = transform.Transform(pos);
    dir = transform.Transform(dir);
}

void Ray::Transform(const float3x4 &transform)
{
    pos = transform.TransformPos(pos);
    dir = transform.TransformDir(dir);
}

void Ray::Transform(const float4x4 &transform)
{
    pos = transform.TransformPos(pos);
    dir = transform.TransformDir(dir);
}

void Ray::Transform(const Quat &transform)
{
    pos = transform.Transform(pos);
    dir = transform.Transform(dir);
}

/// Returns the distance of the given point to this line.
/// @param d [out] This element will receive the distance along this line that specifies the closest point on this line to the given point.
float Ray::Distance(const float3 &point, float *d) const
{
    return ClosestPoint(point, d).Distance(point);
}

/// Returns the distance of the given ray to this line.
/// @param d [out] Receives the distance along this line that specifies the closest point on this line to the given point.
/// @param d2 [out] Receives the distance along the other line that specifies the closest point on that line to this line.
float Ray::Distance(const Ray &other, float *d, float *d2) const
{
    float u2;
    float3 c = ClosestPoint(other, d, &u2);
    if (d2) *d2 = u2;
    return c.Distance(other.GetPoint(u2));
}

float Ray::Distance(const Line &other, float *d, float *d2) const
{
    float u2;
    float3 c = ClosestPoint(other, d, &u2);
    if (d2) *d2 = u2;
    return c.Distance(other.GetPoint(u2));
}

float Ray::Distance(const LineSegment &other, float *d, float *d2) const
{
    float u2;
    float3 c = ClosestPoint(other, d, &u2);
    if (d2) *d2 = u2;
    return c.Distance(other.GetPoint(u2));
}

float3 Ray::ClosestPoint(const float3 &targetPoint, float *d) const
{
    float u = Max(0.f, Dot(targetPoint - pos, dir));
    if (d)
        *d = u;
    return GetPoint(u);
}

float3 Ray::ClosestPoint(const Ray &other, float *d, float *d2) const
{
    return LineLine(pos, pos + dir, other.pos, other.pos + other.dir, d, d2);
}

float3 Ray::ClosestPoint(const Line &other, float *d, float *d2) const
{
    return LineLine(pos, pos + dir, other.pos, other.pos + other.dir, d, d2);
}

float3 Ray::ClosestPoint(const LineSegment &other, float *d, float *d2) const
{
    return LineLine(pos, pos + dir, other.a, other.b, d, d2);
}

Line Ray::ToLine() const
{
    return Line(pos, dir);
}

LineSegment Ray::ToLineSegment(float d) const
{
    return LineSegment(pos, GetPoint(d));
}
