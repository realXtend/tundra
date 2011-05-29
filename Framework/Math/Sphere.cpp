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
/*
Sphere::Sphere(const float3 &pointA, const float3 &pointB, const float3 &pointC)
{
}

Sphere::Sphere(const float3 &pointA, const float3 &pointB, const float3 &pointC, const float3 &pointD)
{
}
*/
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
/*
Sphere Sphere::FastEnclosingSphere(const float3 *pointArray, int numPoints)
Sphere Sphere::ApproximateEnclosingSphere(const float3 *pointArray, int numPoints)
Sphere Sphere::OptimalEnclosingSphere(const float3 *pointArray, int numPoints)

float3 Sphere::RandomPointInside(LCG &rng) const
float3 Sphere::RandomPointOnSurface(LCG &rng) const
*/
float Sphere::Distance(const float3 &point) const
{
    return Max(0.f, pos.Distance(point) - r);
}
/*
float Sphere::Distance(const float3 &point, float3 &outClosestPointOnSphere) const

bool Sphere::Intersect(const Ray &ray, float &outDistance) const
bool Sphere::Intersect(const Line &line, float &outDistance) const
bool Sphere::Intersect(const LineSegment &lineSegment, float &outDistance) const
bool Sphere::Intersect(const AABB &aabb) const
bool Sphere::Intersect(const OBB &obb) const
bool Sphere::Intersect(const Plane &plane) const
bool Sphere::Intersect(const Sphere &sphere) const
bool Sphere::Intersect(const Ellipsoid &ellipsoid) const
bool Sphere::Intersect(const Triangle &triangle) const
bool Sphere::Intersect(const Cylinder &cylinder) const
bool Sphere::Intersect(const Torus &torus) const
bool Sphere::Intersect(const Frustum &frustum) const
bool Sphere::Intersect(const Polygon &polygon) const
bool Sphere::Intersect(const Polyhedron &polyhedron) const

void Sphere::Enclose(const AABB &aabb)
void Sphere::Enclose(const OBB &obb)
void Sphere::Enclose(const Sphere &sphere)
void Sphere::Enclose(const Triangle &triangle)
void Sphere::Enclose(const Polygon &polygon)
bool Sphere::Enclose(const Polyhedron &polyhedron)
void Sphere::Enclose(const LineSegment &lineSegment)
void Sphere::Enclose(const float3 &point)
void Sphere::Enclose(const float3 *pointArray, int numPoints)
*/