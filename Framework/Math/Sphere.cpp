/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"
#include <utility>
#include "MathFunc.h"
#include "OBB.h"
#include "AABB.h"
#include "LCG.h"
#include "LineSegment.h"
#include "Plane.h"
#include "Sphere.h"
#include "float3x3.h"
#include "float3x4.h"
#include "float4.h"
#include "float4x4.h"
#include "Quat.h"

Sphere::Sphere(const float3 &center, float radius)
:pos(center), r(radius) 
{
}

Sphere::Sphere(const float3 &pointA, const float3 &pointB)
{
    pos = (pointA + pointB) / 2.f;
    r = (pointB - pos).Length();
}

Sphere::Sphere(const float3 &pointA, const float3 &pointB, const float3 &pointC)
{
    // See e.g. http://en.wikipedia.org/wiki/Circumcenter .

    float3 b = pointB - pointA;
    float3 c = pointC - pointA;
    float3 normal = Cross(b, c);
    float denom = 2.f * normal.LengthSq();
    if (EqualAbs(denom, 0.f))
    {
        SetNegativeInfinity();
        return;
    }

#if 0
    {
        // The three points are collinear. Construct a line through two most extremal points.
        float dC = Dot(b,c);

        if (dC < 0.f)
            *this = Sphere(pointB, pointC);
        else
        {
            float dB = Dot(b, b);
            if (dC > dB)
                *this = Sphere(pointA, pointC);
            else
                *this = sphere(pointA, pointB);
        }
        return;
    }
#endif

    pos = (c.LengthSq() * Cross(normal, c) + b.LengthSq() * Cross(b, normal)) / denom;
    r = pos.Length();
    pos += pointA;

/* // An alternate formulation that is probably correct, but the above contains fewer operations.
   // This one contains a matrix inverse operation.
    float3x3 m;
    m.SetRow(0, pointB - pointA);
    m.SetRow(1, pointC - pointA);
    m.SetRow(2, Cross(m.Row(0), m.Row(1)));
    float3 lengths = float3(m.Row(0).LengthSq(), m.Row(1).LengthSq(), 0.f) * 0.5f;

    bool success = m.Inverse();
    if (!success)
    {
        SetNegativeInfinity();
        return;
    }

    pos = m * lengths;
    r = pos.Length();
    pos += pointA;
*/
}

Sphere::Sphere(const float3 &pointA, const float3 &pointB, const float3 &pointC, const float3 &pointD)
{
    float3x3 m;
    m.SetRow(0, pointB - pointA);
    m.SetRow(1, pointC - pointA);
    m.SetRow(2, pointD - pointA);
    float3 lengths = float3(m.Row(0).LengthSq(), m.Row(1).LengthSq(), m.Row(2).LengthSq()) * 0.5f;

    bool success = m.Inverse();
    if (!success)
    {
        SetNegativeInfinity();
        return;
    }

    pos = m * lengths;
    r = pos.Length();
    pos += pointA;
}

AABB Sphere::MinimalEnclosingAABB() const
{
    AABB aabb;
    aabb.SetFrom(*this);
    return aabb;
}

AABB Sphere::MaximalContainedAABB() const
{
    AABB aabb;
    aabb.SetCenter(pos, float3(r,r,r));
    return aabb;
}

void Sphere::SetNegativeInfinity()
{
    pos = float3(0,0,0);
    r = -std::numeric_limits<float>::infinity();
}

float Sphere::Volume() const
{
    return 4.f * pi * r*r*r / 3.f;
}

float Sphere::SurfaceArea() const
{
    return 4.f * pi * r*r;
}

bool Sphere::IsFinite() const
{
    return pos.IsFinite() && isfinite(r);
}

bool Sphere::IsDegenerate() const
{
    return r < 0.f;
}

bool Sphere::Contains(const float3 &point) const
{
    return pos.DistanceSq(point) <= r*r;
}

Sphere Sphere::FastEnclosingSphere(const float3 *pts, int numPoints)
{
    Sphere s;
    if (numPoints == 0)
    {
        s.SetNegativeInfinity();
        return s;
    }
    assert(pts);

    // First pass: Pick the cardinal axis (X,Y or Z) which has the two most distant points.
    int minx, maxx, miny, maxy, minz, maxz;
    AABB::ExtremePointsAlongAABB(pts, numPoints, minx, maxx, miny, maxy, minz, maxz);
    float dist2x = pts[minx].DistanceSq(pts[maxx]);
    float dist2y = pts[miny].DistanceSq(pts[maxy]);
    float dist2z = pts[minz].DistanceSq(pts[maxz]);

    int min = minx;
    int max = maxx;
    if (dist2y > dist2x && dist2y > dist2z)
    {
        min = miny;
        max = maxy;
    }
    else if (dist2z > dist2x && dist2z > dist2y)
    {
        min = minz;
        max = maxz;
    }

    // The two points on the longest axis define the initial sphere.
    s.pos = (pts[min] + pts[max]) / 2.f;
    s.r = pts[min].Distance(s.pos);

    // Second pass: Make sure each point lies inside this sphere, expand if necessary.
    for(int i = 0; i < numPoints; ++i)
        s.Enclose(pts[i]);
    return s;
}

/** This implementation was adapted from Christer Ericson's Real-time Collision Detection, pp. 99-100. */
/*
Sphere WelzlSphere(const float3 *pts, int numPoints, float3 *support, int numSupports)
{
    if (numPoints == 0)
    {
        switch(numSupports)
        {
        default: assert(false);
        case 0: return Sphere();
        case 1: return Sphere(support[0], 0.f);
        case 2: return Sphere(support[0], support[1]);
        case 3: return Sphere(support[0], support[1], support[2]);
        case 4: return Sphere(support[0], support[1], support[2], support[3]);
        }
    }

    ///\todo The following recursion can easily crash the stack for large inputs.  Convert this to proper form.
    Sphere smallestSphere = WelzlSphere(pts, numPoints - 1, support, numSupports);
    if (smallestSphere.Contains(pts[numPoints-1]))
        return smallestSphere;
    support[numSupports] = pts[numPoints-1];
    return WelzlSphere(pts, numPoints - 1,  support, numSupports + 1);
}
*/
/*
Sphere Sphere::OptimalEnclosingSphere(const float3 *pts, int numPoints)
{
    float3 support[4];
    WelzlSphere(pts, numPoints, &support, 0);
}
*/
/*
Sphere Sphere::ApproximateEnclosingSphere(const float3 *pointArray, int numPoints)

float3 Sphere::RandomPointInside(LCG &rng) const
float3 Sphere::RandomPointOnSurface(LCG &rng) const
*/
float Sphere::Distance(const float3 &point) const
{
    return Max(0.f, pos.Distance(point) - r);
}

bool Sphere::Intersects(const Sphere &sphere) const
{
    return (pos - sphere.pos).LengthSq() <= r*r + sphere.r*sphere.r;
}

/*
float Sphere::Distance(const float3 &point, float3 &outClosestPointOnSphere) const

bool Sphere::Intersect(const Ray &ray, float &outDistance) const
bool Sphere::Intersect(const Line &line, float &outDistance) const
bool Sphere::Intersect(const LineSegment &lineSegment, float &outDistance) const
bool Sphere::Intersect(const AABB &aabb) const
bool Sphere::Intersect(const OBB &obb) const
bool Sphere::Intersect(const Plane &plane) const
bool Sphere::Intersect(const Ellipsoid &ellipsoid) const
bool Sphere::Intersect(const Triangle &triangle) const
bool Sphere::Intersect(const Cylinder &cylinder) const
bool Sphere::Intersect(const Torus &torus) const
bool Sphere::Intersect(const Frustum &frustum) const
bool Sphere::Intersect(const Polygon &polygon) const
bool Sphere::Intersect(const Polyhedron &polyhedron) const

void Sphere::Enclose(const Triangle &triangle)
void Sphere::Enclose(const Polygon &polygon)
bool Sphere::Enclose(const Polyhedron &polyhedron)
*/

void Sphere::Enclose(const float3 &point)
{
    float3 d = point - pos;
    float dist2 = d.LengthSq();
    if (dist2 > r*r)
    {
        float dist = sqrt(dist2);
        float newRadius = (r + dist) / 2.f;
        pos += d * (newRadius - r) / dist;
        r = newRadius;
    }
}

void Sphere::Enclose(const AABB &aabb)
{
    ///\todo This might not be very optimal at all. Perhaps better to enclose the farthest point first.
    for(int i = 0; i < 8; ++i)
        Enclose(aabb.CornerPoint(i));
}

void Sphere::Enclose(const OBB &obb)
{
    ///\todo This might not be very optimal at all. Perhaps better to enclose the farthest point first.
    for(int i = 0; i < 8; ++i)
        Enclose(obb.CornerPoint(i));
}

void Sphere::Enclose(const Sphere &sphere)
{
    // To enclose another sphere into this sphere, we can simply enclose the farthest point
    // of that sphere to this sphere.
    float3 farthestPoint = sphere.pos - pos;
    farthestPoint = sphere.pos + farthestPoint * (sphere.r / farthestPoint.Length());
    Enclose(farthestPoint);
}

void Sphere::Enclose(const LineSegment &lineSegment)
{
    ///\todo This might not be very optimal at all. Perhaps better to enclose the farthest point first.
    Enclose(lineSegment.a);
    Enclose(lineSegment.b);
}

void Sphere::Enclose(const float3 *pointArray, int numPoints)
{
    ///\todo This might not be very optimal at all. Perhaps better to enclose the farthest point first.
    for(int i = 0; i < numPoints; ++i)
        Enclose(pointArray[i]);
}
