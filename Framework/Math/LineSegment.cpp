/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"

#include "LineSegment.h"
#include "Ray.h"
#include "Line.h"

LineSegment::LineSegment(const float3 &a_, const float3 &b_)
:a(a_), b(b_)
{
}

LineSegment::LineSegment(const Ray &ray, float d)
:a(ray.pos), b(ray.GetPoint(d))
{
}

LineSegment::LineSegment(const Line &line, float d)
:a(line.pos), b(line.GetPoint(d))
{
}

/// Returns a point on the line segment.
float3 LineSegment::GetPoint(float d) const
{
    return (1.f - d) * a + d * b;
}

void LineSegment::Reverse()
{
    std::swap(a, b);
}

float3 LineSegment::Dir() const
{
    return (b - a).Normalized();
}

/// Applies a transformation to this line.
void LineSegment::Transform(const float3x3 &transform)
{
    assume(false && "Not implemented!");
}

void LineSegment::Transform(const float3x4 &transform)
{
    assume(false && "Not implemented!");
}

void LineSegment::Transform(const float4x4 &transform)
{
    assume(false && "Not implemented!");
}

void LineSegment::Transform(const Quat &transform)
{
    assume(false && "Not implemented!");
}

float LineSegment::Length() const
{
    return a.Distance(b);
}

float LineSegment::LengthSq() const
{
    return a.DistanceSq(b);
}

float LineSegment::Distance(const float3 &point) const
{
    assume(false && "Not implemented!");
    return 0.f; ///\todo
}

float LineSegment::Distance(const float3 &point, float &d) const
{
    assume(false && "Not implemented!");
    return 0.f; ///\todo
}

float LineSegment::Distance(const Ray &other, float &d, float &d2) const
{
    assume(false && "Not implemented!");
    return 0.f; ///\todo
}

float LineSegment::Distance(const Line &other, float &d, float &d2) const
{
    assume(false && "Not implemented!");
    return 0.f; ///\todo
}

float LineSegment::Distance(const LineSegment &other, float &d, float &d2) const
{
    assume(false && "Not implemented!");
    return 0.f; ///\todo
}

bool LineSegment::Intersect(const Plane &plane) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const Plane &plane, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const Sphere &sphere) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const Sphere &sphere, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const AABB &aabb) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const AABB &aabb, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const OBB &aabb) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const OBB &aabb, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const Triangle &triangle) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const Triangle &triangle, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const Frustum &frustum) const
{
    assume(false && "Not implemented!");
    return false;
}

bool LineSegment::Intersect(const Frustum &frustum, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const Polyhedron &polyhedron) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

bool LineSegment::Intersect(const Polyhedron &polyhedron, float &outDistance) const
{
    assume(false && "Not implemented!");
    return false; ///\todo
}

Ray LineSegment::ToRay() const
{
    return Ray(a, Dir());
}

Line LineSegment::ToLine() const
{
    return Line(a, Dir());
}
