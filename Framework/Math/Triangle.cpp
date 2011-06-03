/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"

#include "Math/MathFunc.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/Triangle.h"
#include "Math/Plane.h"
#include "Math/Line.h"
#include "Math/LineSegment.h"
#include "Math/Ray.h"

Triangle::Triangle(const float3 &a_, const float3 &b_, const float3 &c_)
:a(a_), b(b_), c(c_)
{
}

float3 Triangle::Barycentric(const float3 &point) const
{
    /// @note An alternate mechanism to compute the barycentric is given in Christer Ericson's
    /// Real-Time Collision Detection, pp. 51-52, which might be slightly faster.
    float3 v0 = b - a;
    float3 v1 = c - a;
    float3 v2 = point - a;
    float d00 = Dot(v0, v0);
    float d01 = Dot(v0, v1);
    float d11 = Dot(v1, v1);
    float d20 = Dot(v2, v0);
    float d21 = Dot(v2, v1);
    float denom = 1.f / (d00 * d11 - d01 * d01);
    float v = (d11 * d20 - d01 * d21) * denom;
    float w = (d00 * d21 - d01 * d20) * denom;
    float u = 1.0f - v - w;
    return float3(u, v, w);
}

bool Triangle::BarycentricInsideTriangle(const float3 &barycentric)
{
    return barycentric.x >= 0.f && barycentric.y >= 0.f && barycentric.z >= 0.f &&
        EqualAbs(barycentric.x + barycentric.y + barycentric.z, 1.f);
}

float3 Triangle::Point(float u, float v, float w) const
{
    return u * a + v * b + w * c;
}

float3 Triangle::Point(const float3 &b) const
{
    return Point(b.x, b.y, b.z);
}

float Triangle::Area() const
{
    return 0.5f * Cross(b-a, c-a).Length();
}

Plane Triangle::GetPlane() const
{
    return Plane(a, b, c);
}

float Triangle::Area2D(const float2 &p1, const float2 &p2, const float2 &p3)
{
    return (p1.x - p2.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p2.y);
}

float Triangle::SignedArea(const float3 &pt, const float3 &a, const float3 &b, const float3 &c)
{
    return Dot(Cross(b-pt, c-pt), Cross(b-a, c-a).Normalized());
}

bool Triangle::IsDegenerate(float epsilon) const
{
    return IsDegenerate(a, b, c);
}

bool Triangle::IsDegenerate(const float3 &a, const float3 &b, const float3 &c, float epsilon)
{
    return a.Equals(b, epsilon) || a.Equals(c, epsilon) || b.Equals(c, epsilon);
}

bool Triangle::Contains(const float3 &point, float triangleThickness) const
{
    if (GetPlane().Distance(point) > triangleThickness)
        return false; ///\todo This test is omitted in Real-Time Collision Detection. p. 25. A bug in the book?

    float3 br = Barycentric(point);
    return br.y >= 0.f && br.z >= 0.f && (br.y + br.z) <= 1.f;
}

bool Triangle::Distance(const float3 &point)
{
    return ClosestPoint(point).Distance(point);
}

bool Triangle::Intersects(const LineSegment &other, float3 *intersectionPoint) const
{
    return false;
}

/// Code from Christer Ericson's Real-Time Collision Detection, pp. 141-142.
float3 Triangle::ClosestPoint(const float3 &p) const
{
    // Check if P is in vertex region outside A.
    float3 ab = b - a;
    float3 ac = c - a;
    float3 ap = p - a;
    float d1 = Dot(ab, ap);
    float d2 = Dot(ac, ap);
    if (d1 <= 0.f && d2 <= 0.f)
        return a; // Barycentric coordinates are (1,0,0).

    // Check if P is in vertex region outside B.
    float3 bp = p - b;
    float d3 = Dot(ab, bp);
    float d4 = Dot(ac, bp);
    if (d3 >= 0.f && d4 <= d3)
        return b; // Barycentric coordinates are (0,1,0).

    // Check if P is in edge region of AB, and if so, return the projection of P onto AB.
    float vc = d1*d4 - d3*d2;
    if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f)
    {
        float v = d1 / (d1 - d3);
        return a + v * ab; // The barycentric coordinates are (1-v, v, 0).
    }

    // Check if P is in vertex region outside C.
    float3 cp = p - c;
    float d5 = Dot(ab, cp);
    float d6 = Dot(ac, cp);
    if (d6 >= 0.f && d5 <= d6)
        return c; // The barycentric coordinates are (0,0,1).

    // Check if P is in edge region of AC, and if so, return the projection of P onto AC.
    float vb = d5*d2 - d1*d6;
    if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f)
    {
        float w = d2 / (d2 - d6);
        return a + w * ac; // The barycentric coordinates are (1-w, 0, w).
    }

    // Check if P is in edge region of BC, and if so, return the projection of P onto BC.
    float va = d3*d6 - d5*d4;
    if (va <= 0.f && d4 - d3 >= 0.f && d5 - d6 >= 0.f)
    {
        float w = (d4 - d3) / (d4 - d3 + d5 - d6);
        return b + w * (c - b); // The barycentric coordinates are (0, 1-w, w).
    }

    // P must be inside the face region. Compute the closest point through its barycentric coordinates (u,v,w).
    float denom = 1.f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return a + ab * v + ac * w;
}

float3 Triangle::ClosestPoint(const LineSegment &line, float3 *otherPt) const
{
    float3 intersectionPoint;
    bool success = Intersects(line, &intersectionPoint);
    if (success)
        return intersectionPoint;

    Plane p = GetPlane();
    float d1 = p.Distance(line.a);
    float d2 = p.Distance(line.b);
    bool aProjectsInsideTriangle = BarycentricInsideTriangle(line.a);
    bool bProjectsInsideTriangle = BarycentricInsideTriangle(line.b);

    if (aProjectsInsideTriangle && bProjectsInsideTriangle)
    {
        // We tested above for intersection, so cannot intersect now.
        if (d1 <= d2)
        {
            if (otherPt)
                *otherPt = line.a;
            return p.Project(line.a);
        }
        else
        {
            if (otherPt)
                *otherPt = line.b;
            return p.Project(line.b);
        }
    }
    LineSegment ab(a, b);
    LineSegment ac(a, c);
    LineSegment bc(b, c);

    float tab, tac, tbc;
    float tab2, tac2, tbc2;

    float dab = ab.Distance(line, &tab, &tab2);
    float dac = ac.Distance(line, &tac, &tac2);
    float dbc = bc.Distance(line, &tbc, &tbc2);

    if (dab <= dac && dab <= dbc && dab <= d1 && dab <= d2)
    {
        if (otherPt)
            *otherPt = line.GetPoint(tab2);
        return ab.GetPoint(tab);
    }
    else if (dac <= dbc && dac <= d1 && dac <= d2)
    {
        if (otherPt)
            *otherPt = line.GetPoint(tac2);
        return ab.GetPoint(tac);
    }
    else if (dbc <= d1 && dbc <= d2)
    {
        if (otherPt)
            *otherPt = line.GetPoint(tbc2);
        return ab.GetPoint(tbc);
    }
    else if (d1 <= d2)
    {
        if (otherPt)
            *otherPt = line.a;
        return p.Project(line.a);
    }
    else
    {
        if (otherPt)
            *otherPt = line.b;
        return p.Project(line.b);
    }
}
