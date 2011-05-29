/** @file Sphere.h
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 

    @brief
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

class Sphere
{
public:
    /// The center point of this sphere.
    float3 pos;
    /// The radius of this sphere.
    float r;

    /// @note The default ctor does not initialize any member values.
    Sphere() {}

    Sphere(const float3 &center, float radius);

    /// Constructs the minimal sphere enclosing the given two points.
    Sphere(const float3 &pointA, const float3 &pointB);

    /// Constructs the minimal sphere enclosing the given three points.
 //   Sphere(const float3 &pointA, const float3 &pointB, const float3 &pointC);

    /// Constructs the minimal sphere enclosing the given four points.
 //   Sphere(const float3 &pointA, const float3 &pointB, const float3 &pointC, const float3 &pointD);

    AABB MinimalEnclosingAABB() const;
    AABB MaximalContainedAABB() const;

    float Volume() const;

    float SurfaceArea() const;

    bool IsFinite() const;

    bool IsDegenerate() const;

    bool Contains(const float3 &point) const;
/*
    static Sphere FastEnclosingSphere(const float3 *pointArray, int numPoints);
    static Sphere ApproximateEnclosingSphere(const float3 *pointArray, int numPoints);
    static Sphere OptimalEnclosingSphere(const float3 *pointArray, int numPoints);

    float3 RandomPointInside(LCG &rng) const;
    float3 RandomPointOnSurface(LCG &rng) const;
*/
    float Distance(const float3 &point) const;
    /*
    float Distance(const float3 &point, float3 &outClosestPointOnSphere) const;

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
    bool Intersect(const Polygon &polygon) const;
    bool Intersect(const Polyhedron &polyhedron) const;
    */
    /*
    void Enclose(const AABB &aabb);
    void Enclose(const OBB &obb);
    void Enclose(const Sphere &sphere);
    void Enclose(const Triangle &triangle);
    void Enclose(const Polygon &polygon);
    bool Enclose(const Polyhedron &polyhedron);
    void Enclose(const LineSegment &lineSegment);
    void Enclose(const float3 &point);
    void Enclose(const float3 *pointArray, int numPoints);
    */
};

#ifdef QT_INTEROP
Q_DECLARE_METATYPE(Sphere)
Q_DECLARE_METATYPE(Sphere*)
#endif
