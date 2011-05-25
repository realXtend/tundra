/** @file Plane.h
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 

    @brief
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

#ifdef OGRE_INTEROP
#include <OgrePlane.h>
#endif

/// Specifies an plane in 3D space. This plane is an affine 2D subspace of the 3D space, meaning
/// that its sides extend to infinity, and it does not necessarily pass through the origin.
class Plane
{
public:
    /// @note The default ctor does not initialize any member values.
    Plane() {}
    /// Constructs a plane by directly specifying the normal and distance parameters.
    /// @param normal The direction the plane is facing. This vector must have been normalized in advance.
    /// @param d The signed distance of this plane from the origin.
    Plane(const float3 &normal, float d);
    /// Constructs a plane by specifying three points on the plane. The normal of the plane will point to 
    /// the halfspace from which the points are observed to be oriented in counter-clockwise order.
    /// @note The points v1, v2 and v3 must not all lie on the same line.
    Plane(const float3 &v1, const float3 &v2, const float3 &v3);
    /// Constructs a plane by specifying a single point on the plane, and the surface normal.
    /// @param normal The direction the plane is facing. This vector must have been normalized in advance.
    Plane(const float3 &point, const float3 &normal);

    /// The direction this plane is facing at.
    float3 normal;
    /// The signed distance of this plane from origin.
    float d;

    /// Sets this plane by specifying three points on the plane. The normal of the plane will point to 
    /// the halfspace from which the points are observed to be oriented in counter-clockwise order.
    /// @note The points v1, v2 and v3 must not all lie on the same line.
    void Set(const float3 &v1, const float3 &v2, const float3 &v3);
    /// Sets this plane by specifying a single point on the plane, and the surface normal.
    /// @param normal The direction the plane is facing. This vector must have been normalized in advance.
    void Set(const float3 &point, const float3 &normal);

    /// Returns a point on this plane.
    /// @note This point has the property that the line passing through the origin and the returned point is
    ///       perpendicular to this plane (directed towards the normal vector of this plane).
    float3 PointOnPlane() const;

    /// Applies a transformation to this plane.
    void Transform(const float3x3 &transform);
    void Transform(const float3x4 &transform);
    void Transform(const float4x4 &transform);
    void Transform(const Quat &transform);

    /// Tests if given direction vector points towards the positive side of this plane.
    bool IsInPositiveDirection(const float3 &directionVector) const;

    /// Tests if given point is on the positive side of this plane.
    bool IsOnPositiveSide(const float3 &point) const;

    /// Tests if two points are on the same side of the plane.
    bool AreOnSameSide(const float3 &p1, const float3 &p2) const;

    /// Returns the distance of this plane to the given point.
    float Distance(const float3 &point) const;

    /// Returns the signed distance of this plane to the given point.
    float SignedDistance(const float3 &point) const;

    /// Returns the affine transformation that projects orthographically onto this plane.
    float3x4 OrthoProjection() const;

    /// Returns the affine transformation that projects onto this plane in an oblique angle.
    float3x4 ObliqueProjection(const float3 &obliqueProjectionDir) const;

    /// Returns the transformation matrix that mirrors objects along this plane.
    float3x4 ReflectionMatrix() const;

    /// Mirrors the given point along this plane.
    float3 Reflect(const float3 &point) const;

    /// Refracts the given normal vector along this plane.
    float3 Refract(const float3 &normal, float negativeSideRefractionIndex, float positiveSideRefractionIndex) const;

    /// Projects the given point onto this plane orthographically (finds the closest point on this plane).
    float3 Project(const float3 &point) const;

    /// Projects the given point onto this plane in the given oblique projection direction.
    float3 ObliqueProject(const float3 &point, const float3 &obliqueProjectionDir) const;

    /// Computes the intersection of two planes.
    bool Intersect(const Plane &plane, Line &outLine) const;
    static bool Intersect(const Plane &plane, const Plane &plane2, Line &outLine);

    /// Computes the intersection of three planes.
    /// @note If two of the planes are identical, the intersection will be a line, but this function does not detect this case, and only returns a single point on the line.
    bool Intersect(const Plane &plane, const Plane &plane2, float3 &outPoint) const;
    static bool Intersect(const Plane &plane, const Plane &plane2, const Plane &plane3, float3 &outPoint);
    
    bool Intersect(const Ray &ray, float *d) const;
    bool Intersect(const Line &line, float *d) const;
    bool Intersect(const LineSegment &lineSegment, float *d) const;
    bool Intersect(const Sphere &sphere) const;
    bool Intersect(const AABB &aabb) const;
    bool Intersect(const OBB &aabb) const;
    bool Intersect(const Triangle &triangle) const;
    bool Intersect(const Frustum &frustum) const;
    bool Intersect(const Polyhedron &polyhedron) const;

    /// Clips a line segment against this plane.
    bool Clip(float3 &a, float3 &b) const;

    /// Clips a line segment against this plane.
    bool Clip(LineSegment &line) const;

    /// Clips a line against this plane.
    bool Clip(const Line &line, Ray &outRay) const;

    /// Clips a triangle against this plane.
    int Clip(const Triangle &triangle, Triangle &t1, Triangle &t2) const;

    /// Tests if two planes are parallel.
    bool AreParallel(const Plane &plane) const;

    /// Returns the angle of intersection between two planes.
    float DihedralAngle(const Plane &plane) const;

    /// Tests if two planes are the same, up to the given epsilon.
    bool Equals(const Plane &other, float epsilon = 1e-3f) const;

    /// Returns a circle that lies on this plane, with its center as close as possible to the specified center point,
    /// and the radius as specified.
    Circle GenerateCircle(const float3 &circleCenter, float radius) const;

//    float3 RandomPointInsideCircle(const float3 &circleCenter, float radius) const;
//    float3 RandomPointOnCircleEdge(const float3 &circleCenter, float radius) const;

#ifdef OGRE_INTEROP
    Plane(const Ogre::Plane &other) { normal = other.normal; d = other.d; }
    operator Ogre::Plane() const { return Ogre::Plane(normal, d); }
#endif
};

Plane operator *(const float3x3 &transform, const Plane &plane);
Plane operator *(const float3x4 &transform, const Plane &plane);
Plane operator *(const float4x4 &transform, const Plane &plane);
Plane operator *(const Quat &transform, const Plane &plane);
