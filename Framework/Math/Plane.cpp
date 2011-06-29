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

Plane::Plane(const Ray &ray, const float3 &normal)
{
	float3 perpNormal = normal - normal.ProjectToNorm(ray.dir);
	Set(ray.pos, perpNormal.Normalized());
}

Plane::Plane(const Line &line, const float3 &normal)
{
	float3 perpNormal = normal - normal.ProjectToNorm(line.dir);
	Set(line.pos, perpNormal.Normalized());
}

Plane::Plane(const LineSegment &lineSegment, const float3 &normal)
{
	float3 perpNormal = normal - normal.ProjectTo(lineSegment.b - lineSegment.a);
	Set(lineSegment.a, perpNormal.Normalized());
}

void Plane::Set(const float3 &v1, const float3 &v2, const float3 &v3)
{
    assume(!Line::AreCollinear(v1, v2, v3));
    normal = (v2-v1).Cross(v3-v1).Normalized();
    d = Dot(v1, normal);
}

void Plane::Set(const float3 &point, const float3 &normal_)
{
    normal = normal_;
    assume(normal.IsNormalized());
    d = Dot(point, normal);
}

float3 Plane::PointOnPlane() const
{
    return normal * d;
}

void Plane::Transform(const float3x3 &transform)
{
    float3x3 it = transform.InverseTransposed(); ///\todo Could optimize the inverse here by assuming orthogonality or orthonormality.
    normal = it * normal;
}

/// See Eric Lengyel's Mathematics for 3D Game Programming And Computer Graphics 2nd ed., p.110, chapter 4.2.3.
void Plane::Transform(const float3x4 &transform)
{
    ///\todo Could optimize this function by switching to plane convention ax+by+cz+d=0 instead of ax+by+cz=d.
    float3x3 r = transform.Float3x3Part();
    bool success = r.Inverse(); ///\todo Can optimize the inverse here by assuming orthogonality or orthonormality.
    assume(success);
    d = d + Dot(normal, r * transform.TranslatePart());
    normal = normal * r;
}

void Plane::Transform(const float4x4 &transform)
{
    assume(transform.Row(3).Equals(float4(0,0,0,1)));
    Transform(transform.Float3x4Part());
}

void Plane::Transform(const Quat &transform)
{
    float3x3 r = transform.ToFloat3x3();
    Transform(r);
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
    return normal.Dot(point) - d;
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
    return point - (Dot(normal, point) - d) * normal;
}

float3 Plane::ObliqueProject(const float3 &point, const float3 &obliqueProjectionDir) const
{
    assume(false && "Not implemented!"); ///\todo
    return float3();
}

bool Plane::Contains(const float3 &point, float distanceThreshold) const
{
    return Distance(point) <= distanceThreshold;
}

bool Plane::Intersects(const Plane &plane, Line *outLine) const
{
    float3 perp = Cross(normal, plane.normal);

    float3x3 m;
    m.SetRow(0, normal);
    m.SetRow(1, plane.normal);
    m.SetRow(2, perp); // This is arbitrarily chosen, to produce m invertible.
    bool success = m.Inverse();
    if (!success) // Inverse failed, so the planes must be parallel.
    {
        if (EqualAbs(d, plane.d)) // The planes are equal?
        {
            if (outLine)
                *outLine = Line(plane.PointOnPlane(), plane.normal.Perpendicular());
            return true;
        }
        else
            return false;
    }
    if (outLine)
        *outLine = Line(m * float3(d, plane.d, 0.f), perp.Normalized());
    return true;
}

bool Plane::Intersects(const Plane &plane, const Plane &plane2, Line *outLine, float3 *outPoint) const
{
    Line dummy;
    if (!outLine)
        outLine = &dummy;

    // First check all planes for parallel pairs.
    if (this->IsParallel(plane) || this->IsParallel(plane2))
    {
        if (EqualAbs(d, plane.d) || EqualAbs(d, plane2.d))
        {
            bool intersect = plane.Intersects(plane2, outLine);
            if (intersect && outPoint)
                *outPoint = outLine->GetPoint(0);
            return intersect;
        }
        else
            return false;
    }
    if (plane.IsParallel(plane2))
    {
        if (EqualAbs(plane.d, plane2.d))
        {
            bool intersect = this->Intersects(plane, outLine);
            if (intersect && outPoint)
                *outPoint = outLine->GetPoint(0);
            return intersect;
        }
        else
            return false;
    }

    // All planes point to different directions.
    float3x3 m;
    m.SetRow(0, normal);
    m.SetRow(1, plane.normal);
    m.SetRow(2, plane2.normal);
    bool success = m.Inverse();
    if (!success)
        return false;
    if (outPoint)
        *outPoint = m * float3(d, plane.d, plane2.d);
    return true;
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

bool Plane::Intersects(const Ray &ray, float *d) const
{
    float t;
    bool success = IntersectLinePlane(PointOnPlane(), normal, ray.pos, ray.dir, &t);
    if (d)
        *d = t;
    return success && t >= 0.f;
}

bool Plane::Intersects(const Line &line, float *d) const
{
    return IntersectLinePlane(PointOnPlane(), normal, line.pos, line.dir, d);
}

bool Plane::Intersects(const LineSegment &lineSegment, float *d) const
{
    float t;
    bool success = IntersectLinePlane(PointOnPlane(), normal, lineSegment.a, lineSegment.Dir(), &t);
    const float lineSegmentLength = lineSegment.Length();
    if (d)
        *d = t / lineSegmentLength;
    return success && t >= 0.f && t <= lineSegmentLength;
}

bool Plane::Intersects(const Sphere &sphere) const
{
    return Distance(sphere.pos) <= sphere.r;
}

/// Set Christer Ericson's Real-Time Collision Detection, p.164.
bool Plane::Intersects(const AABB &aabb) const
{
    float3 c = aabb.CenterPoint();
    float3 e = aabb.HalfDiagonal();

    // Compute the projection interval radius of the AABB onto L(t) = aabb.center + t * plane.normal;
    float r = e[0]*Abs(normal[0]) + e[1]*Abs(normal[1]) + e[2]*Abs(normal[2]);
    // Compute the distance of the box center from plane.
    float s = Dot(normal, c) - d;
    return Abs(s) <= r;
}

bool Plane::Intersects(const OBB &obb) const
{
    return obb.Intersects(*this);
}

bool Plane::Intersects(const Triangle &triangle) const
{
    float a = SignedDistance(triangle.a);
    float b = SignedDistance(triangle.b);
    float c = SignedDistance(triangle.c);
    return (a*b <= 0.f || a*c <= 0.f);
}

bool Plane::Intersects(const Frustum &frustum) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}
/*
bool Plane::Intersects(const Polyhedron &polyhedron) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}
*/
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

bool Plane::IsParallel(const Plane &plane, float epsilon) const
{
    return normal.Equals(plane.normal, epsilon);
}

float Plane::DihedralAngle(const Plane &plane) const
{
    assume(false && "Not implemented!"); ///\todo
    return false;
}

bool Plane::Equals(const Plane &other, float epsilon) const
{
    return IsParallel(other, epsilon) && EqualAbs(d, other.d, epsilon);
}

Circle Plane::GenerateCircle(const float3 &circleCenter, float radius) const
{
    assume(false && "Not implemented!"); ///\todo
    return Circle();
}

Plane operator *(const float3x3 &transform, const Plane &plane)
{
    Plane p(plane);
    p.Transform(transform);
    return p;
}

Plane operator *(const float3x4 &transform, const Plane &plane)
{
    Plane p(plane);
    p.Transform(transform);
    return p;
}

Plane operator *(const float4x4 &transform, const Plane &plane)
{
    Plane p(plane);
    p.Transform(transform);
    return p;
}

Plane operator *(const Quat &transform, const Plane &plane)
{
    Plane p(plane);
    p.Transform(transform);
    return p;
}
