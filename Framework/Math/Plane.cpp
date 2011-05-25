/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"

#include "Circle.h"
#include "MathFunc.h"
#include "Plane.h"
#include "Line.h"
#include "Ray.h"
#include "LineSegment.h"
#include "float3x4.h"

Plane::Plane(const float3 &normal_, float d_)
:normal(normal_), d(d_)
{
    assume(normal.IsNormalized());
}

Plane::Plane(const float3 &v1, const float3 &v2, const float3 &v3)
{
    Set(v1, v2, v3);
}

Plane::Plane(const float3 &point, const float3 &normal_)
{
    Set(point, normal_);
}

void Plane::Set(const float3 &v1, const float3 &v2, const float3 &v3)
{
    assume(!Line::AreCollinear(v1, v2, v3));
    normal = (v2-v1).Cross(v3-v1).Normalized();
    d = -Dot(v1, normal);
}

void Plane::Set(const float3 &point, const float3 &normal_)
{
    normal = normal_;
    assume(normal.IsNormalized());
    d = -Dot(point, normal);
}

float3 Plane::PointOnPlane() const
{
    return normal * -d;
}

void Plane::Transform(const float3x3 &transform)
{
    assume(false && "Not implemented!"); ///\todo
}

void Plane::Transform(const float3x4 &transform)
{
    assume(false && "Not implemented!"); ///\todo
}

void Plane::Transform(const float4x4 &transform)
{
    assume(false && "Not implemented!"); ///\todo
}

void Plane::Transform(const Quat &transform)
{
    assume(false && "Not implemented!"); ///\todo
}

bool Plane::IsInPositiveDirection(const float3 &directionVector) const
{
    assume(directionVector.IsNormalized());
    return normal.Dot(directionVector) >= 0.f;
}

bool Plane::IsOnPositiveSide(const float3 &point) const
{
    return SignedDistance(point) >= 0.f;
}

bool Plane::AreOnSameSide(const float3 &p1, const float3 &p2) const
{
    return SignedDistance(p1) * SignedDistance(p2) >= 0.f;
}

float Plane::Distance(const float3 &point) const
{
    return abs(SignedDistance(point));
}

float Plane::SignedDistance(const float3 &point) const
{
    return normal.Dot(point) + d;
}

float3x4 Plane::OrthoProjection() const
{
    assume(false && "Not implemented!"); ///\todo
    return float3x4();
}

float3x4 Plane::ObliqueProjection(const float3 &obliqueProjectionDir) const
{
    assume(false && "Not implemented!"); ///\todo
    return float3x4();
}

float3x4 Plane::ReflectionMatrix() const
{
    assume(false && "Not implemented!"); ///\todo
    return float3x4();
}

float3 Plane::Reflect(const float3 &point) const
{
    assume(false && "Not implemented!"); ///\todo
    return float3();
}

float3 Plane::Refract(const float3 &normal, float negativeSideRefractionIndex, float positiveSideRefractionIndex) const
{
    assume(false && "Not implemented!"); ///\todo
    return float3();
}

float3 Plane::Project(const float3 &point) const
{
    assume(false && "Not implemented!"); ///\todo
    return float3();
}

float3 Plane::ObliqueProject(const float3 &point, const float3 &obliqueProjectionDir) const
{
    assume(false && "Not implemented!"); ///\todo
    return float3();
}

bool Plane::Intersect(const Plane &plane, Line &outLine) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Intersect(const Plane &plane, const Plane &plane2, Line &outLine)
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Intersect(const Plane &plane, const Plane &plane2, float3 &outPoint) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Intersect(const Plane &plane, const Plane &plane2, const Plane &plane3, float3 &outPoint)
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool IntersectLinePlane(const float3 &ptOnPlane, const float3 &planeNormal, const float3 &lineStart, const float3 &lineDir, float *t)
{
    float denom = Dot(lineDir, planeNormal);
    if (EqualAbs(denom, 0.f))
        return false; // Either we have no intersection, or the whole line is on the plane. \todo distinguish these cases.
    if (t)
        *t = Dot(ptOnPlane - lineStart, planeNormal);
    return true;
}

bool Plane::Intersect(const Ray &ray, float *d) const
{
    float t;
    bool success = IntersectLinePlane(PointOnPlane(), normal, ray.pos, ray.dir, &t);
    if (d)
        *d = t;
    return success && t >= 0.f;
}

bool Plane::Intersect(const Line &line, float *d) const
{
    return IntersectLinePlane(PointOnPlane(), normal, line.pos, line.dir, d);
}

bool Plane::Intersect(const LineSegment &lineSegment, float *d) const
{
    float t;
    bool success = IntersectLinePlane(PointOnPlane(), normal, lineSegment.a, lineSegment.Dir(), &t);
    const float lineSegmentLength = lineSegment.Length();
    if (d)
        *d = t / lineSegmentLength;
    return success && t >= 0.f && t <= lineSegmentLength;
}

bool Plane::Intersect(const Sphere &sphere) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Intersect(const AABB &aabb) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Intersect(const OBB &aabb) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Intersect(const Triangle &triangle) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Intersect(const Frustum &frustum) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Intersect(const Polyhedron &polyhedron) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Clip(float3 &a, float3 &b) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Clip(LineSegment &line) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Clip(const Line &line, Ray &outRay) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

int Plane::Clip(const Triangle &triangle, Triangle &t1, Triangle &t2) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::AreParallel(const Plane &plane) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

float Plane::DihedralAngle(const Plane &plane) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Equals(const Plane &other, float epsilon) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

Circle Plane::GenerateCircle(const float3 &circleCenter, float radius) const
{
    assume(false && "Not implemented!"); ///\todo
    return Circle();
}
