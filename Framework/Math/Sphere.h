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

    /// Sets pos = (0,0,0) and r = -inf.
    void SetNegativeInfinity();

    float Volume() const;

    float SurfaceArea() const;

    bool IsFinite() const;

    bool IsDegenerate() const;

    bool Contains(const float3 &point) const;

    /// Returns a Sphere that bounds the given point array.
    /// This functions implements a fast approximate (though rather crude) algorithm of Jack Ritter.
    /// See "An Efficient Bounding Sphere", in Graphics Gems 1, pp. 301-303,
    /// or Christer Ericson's Real-time Collision Detection, pp. 89-91.
    /// This algorithm performs two linear passes over the data set, i.e. it is O(n).
    static Sphere FastEnclosingSphere(const float3 *pointArray, int numPoints);

/*
    static Sphere ApproximateEnclosingSphere(const float3 *pointArray, int numPoints);
    static Sphere OptimalEnclosingSphere(const float3 *pointArray, int numPoints);

    float3 RandomPointInside(LCG &rng) const;
    float3 RandomPointOnSurface(LCG &rng) const;
*/
    float Distance(const float3 &point) const;
    /*
    float Distance(const float3 &point, float3 &outClosestPointOnSphere) const;

    bool Intersects(const Ray &ray, float &outDistance) const;
    bool Intersects(const Line &line, float &outDistance) const;
    bool Intersects(const LineSegment &lineSegment, float &outDistance) const;
    bool Intersects(const AABB &aabb) const;
    bool Intersects(const OBB &obb) const;
    bool Intersects(const Plane &plane) const;
    bool Intersects(const Ellipsoid &ellipsoid) const;
    bool Intersects(const Triangle &triangle) const;
    bool Intersects(const Cylinder &cylinder) const;
    bool Intersects(const Torus &torus) const;
    bool Intersects(const Frustum &frustum) const;
    bool Intersects(const Polygon &polygon) const;
    bool Intersects(const Polyhedron &polyhedron) const;
    */
    bool Intersects(const Sphere &sphere) const;

    void Enclose(const AABB &aabb);
    void Enclose(const OBB &obb);
    void Enclose(const Sphere &sphere);
//    void Enclose(const Triangle &triangle);
//    void Enclose(const Polygon &polygon);
//    bool Enclose(const Polyhedron &polyhedron);
    void Enclose(const LineSegment &lineSegment);
    void Enclose(const float3 &point);
    void Enclose(const float3 *pointArray, int numPoints);
};

#ifdef QT_INTEROP
Q_DECLARE_METATYPE(Sphere)
Q_DECLARE_METATYPE(Sphere*)
#endif
