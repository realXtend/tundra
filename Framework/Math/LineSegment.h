/** @file LineSegment.h
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 

    @brief
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

class LineSegment
{
public:
    /// @note The default ctor does not initialize any member values.
    LineSegment() {}
    LineSegment(const float3 &a, const float3 &b);
    explicit LineSegment(const Ray &ray, float d);
    explicit LineSegment(const Line &line, float d);

    /// The start point of this line segment.
    float3 a;
    /// The end point of this line segment.
    float3 b;

    /// Returns a point on the line.
    /// @param d The normalized distance along the line segment to compute. This is in the range [0, 1].
    /// @important The meaning of d here differs from Line::GetPoint and Ray::GetPoint. GetPoint(0) returns a, GetPoint(1) returns b.
    float3 GetPoint(float d) const;

    /// Reverses this line segment in place. That is, swaps the start and end points of
    /// this line segment so that it goes from end->start now.
    void Reverse();

    /// Returns the normalized direction vector that points in the direction a->b.
    float3 Dir() const;

    /// Applies a transformation to this line.
    void Transform(const float3x3 &transform);
    void Transform(const float3x4 &transform);
    void Transform(const float4x4 &transform);
    void Transform(const Quat &transform);

    float Length() const;
    float LengthSq() const;

    float Distance(const float3 &point) const;
    float Distance(const float3 &point, float &d) const;
    float Distance(const Ray &other, float &d, float &d2) const;
    float Distance(const Line &other, float &d, float &d2) const;
    float Distance(const LineSegment &other, float &d, float &d2) const;

    bool Intersect(const Plane &plane) const;
    bool Intersect(const Plane &plane, float &outDistance) const;

    bool Intersect(const Sphere &sphere) const;
    bool Intersect(const Sphere &sphere, float &outDistance) const;

    bool Intersect(const AABB &aabb) const;
    bool Intersect(const AABB &aabb, float &outDistance) const;

    bool Intersect(const OBB &aabb) const;
    bool Intersect(const OBB &aabb, float &outDistance) const;

    bool Intersect(const Triangle &triangle) const;
    bool Intersect(const Triangle &triangle, float &outDistance) const;

    bool Intersect(const Frustum &frustum) const;
    bool Intersect(const Frustum &frustum, float &outDistance) const;

//    bool Intersect(const Polyhedron &polyhedron) const;
//    bool Intersect(const Polyhedron &polyhedron, float &outDistance) const;

    Ray ToRay() const;
    Line ToLine() const;
};

LineSegment operator *(const float3x3 &transform, const LineSegment &line);
LineSegment operator *(const float3x4 &transform, const LineSegment &line);
LineSegment operator *(const float4x4 &transform, const LineSegment &line);
LineSegment operator *(const Quat &transform, const LineSegment &line);

#ifdef QT_INTEROP
Q_DECLARE_METATYPE(LineSegment)
Q_DECLARE_METATYPE(LineSegment*)
#endif
