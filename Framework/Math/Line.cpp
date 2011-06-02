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

/// This code is adapted from http://paulbourke.net/geometry/lineline3d/ .
/// dmnop = (xm - xn)(xo - xp) + (ym - yn)(yo - yp) + (zm - zn)(zo - zp).
/// @param v An array of four floats: [0]: line 0 start. [1]: line 0 end. [2]: line 1 start. [3]: line 1 end.
float Dmnop(const float3 *v, int m, int n, int o, int p)
{
    return (v[m].x - v[n].x) * (v[o].x - v[p].x) + (v[m].y - v[n].y) * (v[o].y - v[p].y) + (v[m].z - v[n].z) * (v[o].z - v[p].z);
}

/// Computes the closest points on both lines start0<->end0 and start1<->end1 to each other.
/// @param d [out] The normalize distance along the first line that specifies the closest point is returned here.
/// @param d2 [out] The normalize distance along the second line that specifies the closest point is returned here.
/// @return Returns the closest point on line start0<->end0 to the second line.
float3 LineLine(float3 start0, float3 end0, float3 start1, float3 end1, float *d, float *d2)
{
    const float3 v[4] = { start0, end0, start1, end1 };

    float d0232 = Dmnop(v,0,2,3,2);
    float d3210 = Dmnop(v,3,2,1,0);
    float d3232 = Dmnop(v,3,2,3,2);
    float mu = (d0232 * d3210 - Dmnop(v,0,2,1,0)*d3232) / (Dmnop(v,1,0,1,0)*Dmnop(v,3,2,3,2) - Dmnop(v,3,2,1,0)*Dmnop(v,3,2,1,0));
    if (d)
        *d = mu;

    if (d2)
        *d2 = (d0232 + mu * d3210) / d3232;

    return start0 + mu * (end0 - start0);
}

Line::Line(const float3 &pos_, const float3 &dir_)
:pos(pos_), dir(dir_)
{
    assume(dir.IsNormalized());
}

Line::Line(const Ray &ray)
:pos(ray.pos), dir(ray.dir)
{
    assume(dir.IsNormalized());
}

Line::Line(const LineSegment &lineSegment)
:pos(lineSegment.a), dir(lineSegment.Dir())
{
}

/// Returns a point on this line.
float3 Line::GetPoint(float d) const
{
    assert(dir.IsNormalized());
    return pos + d * dir;
}

/// Applies a transformation to this line.
void Line::Transform(const float3x3 &transform)
{
    pos = transform.Transform(pos);
    dir = transform.Transform(dir);
}

void Line::Transform(const float3x4 &transform)
{
    pos = transform.TransformPos(pos);
    dir = transform.TransformDir(dir);
}

void Line::Transform(const float4x4 &transform)
{
    pos = transform.TransformPos(pos);
    dir = transform.TransformDir(dir);
}

void Line::Transform(const Quat &transform)
{
    pos = transform.Transform(pos);
    dir = transform.Transform(dir);
}

/// Returns the distance of the given point to this line.
/// @param d [out] This element will receive the distance along this line that specifies the closest point on this line to the given point.
float Line::Distance(const float3 &point, float *d) const
{
    return ClosestPoint(point, d).Distance(point);
}

/// Returns the distance of the given ray to this line.
/// @param d [out] Receives the distance along this line that specifies the closest point on this line to the given point.
/// @param d2 [out] Receives the distance along the other line that specifies the closest point on that line to this line.
float Line::Distance(const Ray &other, float *d, float *d2) const
{
    float u2;
    float3 c = ClosestPoint(other, d, &u2);
    if (d2) *d2 = u2;
    return c.Distance(other.GetPoint(u2));
}

float Line::Distance(const Line &other, float *d, float *d2) const
{
    float u2;
    float3 c = ClosestPoint(other, d, &u2);
    if (d2) *d2 = u2;
    return c.Distance(other.GetPoint(u2));
}

float Line::Distance(const LineSegment &other, float *d, float *d2) const
{
    float u2;
    float3 c = ClosestPoint(other, d, &u2);
    if (d2) *d2 = u2;
    return c.Distance(other.GetPoint(u2));
}

float3 Line::ClosestPoint(const float3 &targetPoint, float *d) const
{
    float u = Dot(targetPoint - pos, dir);
    if (d)
        *d = u;
    return GetPoint(u);
}

float3 Line::ClosestPoint(const Ray &other, float *d, float *d2) const
{
    ///\bug Properly cap d2.
    return LineLine(pos, pos + dir, other.pos, other.pos + other.dir, d, d2);
}

float3 Line::ClosestPoint(const Line &other, float *d, float *d2) const
{
    return LineLine(pos, pos + dir, other.pos, other.pos + other.dir, d, d2);
}

float3 Line::ClosestPoint(const LineSegment &other, float *d, float *d2) const
{
    ///\bug Properly cap d2.
    return LineLine(pos, pos + dir, other.a, other.b, d, d2);
}

bool Line::AreCollinear(const float3 &p1, const float3 &p2, const float3 &p3, float epsilon)
{
    ///\todo Improve this check to be distance length -invariant.
    return (p2-p1).Dot(p3-p1) <= epsilon;
}

Ray Line::ToRay() const
{
    return Ray(pos, dir);
}

LineSegment Line::ToLineSegment(float d) const
{
    return LineSegment(pos, GetPoint(d));
}
