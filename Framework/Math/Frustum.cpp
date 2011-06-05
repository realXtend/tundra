/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"

#include "AABB.h"
#include "Circle.h"
#include "MathFunc.h"
#include "Frustum.h"
#include "Plane.h"
#include "Line.h"
#include "OBB.h"
#include "Ray.h"
#include "Sphere.h"
#include "Triangle.h"
#include "LineSegment.h"
#include "float3x3.h"
#include "float3x4.h"
#include "float4.h"
#include "Quat.h"

float Frustum::AspectRatio() const
{
    return horizontalFov / verticalFov;
}

Plane Frustum::NearPlane() const
{
    return Plane(pos + front * nearPlaneDistance, -front);
}

Plane Frustum::FarPlane() const
{
    return Plane(pos + front * farPlaneDistance, front);
}

Plane Frustum::LeftPlane() const
{
    float3 left = Cross(up, front);
    left.ScaleToLength(tan(horizontalFov*0.5f));
    float3 leftSide = front + left;
    float3 leftSideNormal = Cross(up, leftSide).Normalized();
    return Plane(pos, leftSideNormal);
}

Plane Frustum::RightPlane() const
{
    float3 right = Cross(front, up);
    right.ScaleToLength(tan(horizontalFov*0.5f));
    float3 rightSide = front + right;
    float3 rightSideNormal = Cross(rightSide, up).Normalized();
    return Plane(pos, rightSideNormal);
}

Plane Frustum::TopPlane() const
{
    float3 topSide = front + tan(verticalFov * 0.5f) * up;
    float3 right = Cross(front, up);
    float3 topSideNormal = Cross(right, topSide).Normalized();
    return Plane(pos, topSideNormal);
}

Plane Frustum::BottomPlane() const
{
    float3 bottomSide = front - tan(verticalFov * 0.5f) * up;
    float3 left = Cross(up, front);
    float3 bottomSideNormal = Cross(left, bottomSide).Normalized();
    return Plane(pos, bottomSideNormal);
}

float4x4 Frustum::ProjectionMatrix() const
{
    assume(false && "Not implemented!");
    return float4x4();
}

float3 Frustum::NearPlanePos(float x, float y) const
{
    float frontPlaneWidth = 2.f * tan(horizontalFov*0.5f)*nearPlaneDistance;
    float frontPlaneHeight = 2.f * tan(verticalFov*0.5f)*nearPlaneDistance;
    x = (x - 0.5f) * 2.f * frontPlaneWidth;
    y = (y - 0.5f) * 2.f * frontPlaneHeight;
    float3 right = Cross(front, up).Normalized();
    return pos + front * nearPlaneDistance + x * right - y * up;
}

float3 Frustum::FarPlanePos(float x, float y) const
{
    float farPlaneWidth = 2.f * tan(horizontalFov*0.5f)*farPlaneDistance;
    float farPlaneHeight = 2.f * tan(verticalFov*0.5f)*farPlaneDistance;
    x = (x - 0.5f) * 2.f * farPlaneWidth;
    y = (y - 0.5f) * 2.f * farPlaneHeight;
    float3 right = Cross(front, up).Normalized();
    return pos + front * farPlaneDistance + x * right - y * up;
}

Ray Frustum::LookAt(float x, float y) const
{
    float3 nearPlanePos = NearPlanePos(x, y);
    return Ray(pos, (nearPlanePos - pos).Normalized());
}

bool Frustum::Contains(const float3 &point) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::IsFinite() const
{
    return pos.IsFinite() && front.IsFinite() && up.IsFinite() && isfinite(nearPlaneDistance)
        && isfinite(farPlaneDistance) && isfinite(horizontalFov) && isfinite(verticalFov);
}

bool Frustum::IsDegenerate() const
{
    assume(false && "Not implemented!");
    return false;
}

Plane Frustum::GetPlane(int faceIndex) const
{
    assume(0 <= faceIndex && faceIndex <= 5);
    switch(faceIndex)
    {
        default: // For release builds where assume() is disabled, return always the first option if out-of-bounds.
        case 0: return NearPlane();
        case 1: return FarPlane();
        case 2: return LeftPlane();
        case 3: return RightPlane();
        case 4: return TopPlane();
        case 5: return BottomPlane();
    }
}

float Frustum::Volume() const
{
    assume(false && "Not implemented!");
    return -1.f;
}

float3 Frustum::RandomPointInside(LCG &rng) const
{
    assume(false && "Not implemented!");
    return float3();
}

void Frustum::Translate(const float3 &offset)
{
    pos += offset;
}

void Frustum::Scale(const float3 &centerPoint, float uniformScaleFactor)
{
    assume(false && "Not implemented!");
}

void Frustum::Scale(const float3 &centerPoint, const float3 &nonuniformScaleFactors)
{
    assume(false && "Not implemented!");
}

void Frustum::Transform(const float3x3 &transform)
{
    assume(false && "Not implemented!");
}

void Frustum::Transform(const float3x4 &transform)
{
    assume(false && "Not implemented!");
}

void Frustum::Transform(const float4x4 &transform)
{
    assume(false && "Not implemented!");
}

void Frustum::Transform(const Quat &transform)
{
    assume(false && "Not implemented!");
}

void Frustum::GetPlanes(Plane *outArray) const
{
    assume(false && "Not implemented!");
}

void Frustum::GetCornerPoints(float3 *outPointArray) const
{
    assume(false && "Not implemented!");
}

float3 Frustum::CornerPoint(int cornerIndex) const
{
    assume(false && "Not implemented!");
    return float3();
}

AABB Frustum::ToAABB() const
{
    assume(false && "Not implemented!");
    return AABB();
}

OBB Frustum::ToOBB() const
{
    assume(false && "Not implemented!");
    return OBB();
}
/*
Polyhedron Frustum::ToPolyhedron() const
{
    assume(false && "Not implemented!");
    return Polyhedron();
}
*/
bool Frustum::Intersects(const Ray &ray, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const Line &line, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const LineSegment &lineSegment, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const AABB &aabb) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const OBB &obb) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const Plane &plane) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const Sphere &sphere) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const Ellipsoid &ellipsoid) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const Triangle &triangle) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const Cylinder &cylinder) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const Torus &torus) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const Frustum &frustum) const
{
    assume(false && "Not implemented!");
    return false;
}

bool Frustum::Intersects(const Polyhedron &polyhedron) const
{
    assume(false && "Not implemented!");
    return false;
}
