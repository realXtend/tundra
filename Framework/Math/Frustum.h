/** @file Frustum.h
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 

    @brief
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

class Frustum
{
public:
    Frustum() {}

    float3 eyePoint;
    float3 lookDirection;
    float nearPlaneDistance;
    float farPlaneDistance;
    float horizontalFov;
    float verticalFov;

    float AspectRatio() const;
    Plane NearPlane() const;
    Plane FarPlane() const;
    Plane LeftPlane() const;
    Plane RightPlane() const;
    Plane TopPlane() const;
    Plane BottomPlane() const;

    float4x4 ProjectionMatrix() const;

    /// Finds a ray that originates at the eye point and looks in the given direction inside the frustum.
    Ray Lookat(float x, float y) const;

    /// Tests if a point is inside the frustum.
    bool Contains(const float3 &point) const;

    /// Returns true if the elements in this data structure contain valid finite float values.
    bool IsFinite() const;

    bool IsDegenerate() const;

    Plane GetPlane(int faceIndex) const;

    float Volume() const;

    float3 RandomPointInside(LCG &rng) const;

    /// Translates this AABB by the given amount.
    void Translate(const float3 &offset);

    /// Scales this AABB by with respect to the given center point.
    void Scale(const float3 &centerPoint, float scaleFactor);
    void Scale(const float3 &centerPoint, const float &scaleFactor);

    void Transform(const float3x3 &transform);
    void Transform(const float3x4 &transform);
    void Transform(const float4x4 &transform);
    void Transform(const Quat &transform);

    /// Returns all the frustum planes in order (near, far, left, right, top, bottom).
    void GetPlanes(Plane *outArray) const;

    void GetCornerPoints(float3 *outPointArray) const;

    float3 CornerPoint(int cornerIndex) const;

    /// Returns the minimal enclosing AABB of this frustum.
    AABB ToAABB() const;

    /// Returns an OBB that encloses this frustum.
    OBB ToOBB() const;

    /// Returns an exact polyhedron representation of this frustum.
    Polyhedron ToPolyhedron() const;

    bool Intersect(const Ray &ray, float &outDistance) const;
    bool Intersect(const Line &line, float &outDistance) const;
    bool Intersect(const LineSegment &lineSegment, float &outDistance) const;
    bool Intersect(const AABB &aabb) const;
    bool Intersect(const OBB &obb) const;
    bool Intersect(const Plane &plane) const;
    bool Intersect(const Sphere &sphere) const;
    bool Intersect(const Ellipsoid &ellipsoid) const;
    bool Intersect(const Triangle &triangle) const;
    bool Intersect(const Cylinder &cylinder) const;
    bool Intersect(const Torus &torus) const;
    bool Intersect(const Frustum &frustum) const;
    bool Intersect(const Polyhedron &polyhedron) const;
};
