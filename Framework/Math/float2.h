/** @file float2.h
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 

    @brief A 2D (x,y) ordered pair.
*/
#pragma once

#include <string>
#include "MathFwd.h"

#ifdef QT_INTEROP
#include <QVector2D>
#endif

#ifdef OGRE_INTEROP
#include <OgreVector2.h>
#endif

/// A vector of form (x,y).
class float2
{
public:
    enum { Size = 2 };
    float x;
    float y;

    /// This ctor does not initialize the x & y members with a value.
    float2() {}
    /// Initializes to (x, y).
    float2(float x, float y);

    /// Constructs this float2 from an array. The array must contain at least 2 elements.
    float2(const float *data);

    /// Returns a pointer to first float2 element. The data is contiguous in memory.
    float *ptr();
    /// Returns a pointer to first float2 element. The data is contiguous in memory.
    const float *ptr() const;

    /// Accesses the elements (x,y) using array notation. x: *this[0], y: *this[1].
    const float operator [](int index) const;

    /// Accesses the elements (x,y) using array notation. x: *this[0], y: *this[1].
    float &operator [](int index);

    /// Returns the squared length of this vector. Calling this function is faster than calling Length(), so e.g.
    /// instead of comparing the lengths of vectors, you can compare the squared lengths, and avoid expensive square roots.
    float LengthSq() const;
    /// Returns the length of this vector.
    float Length() const;
    /// Normalizes this float2.
    /// Returns the old length of this vector, or 0 if normalization failed.
    float Normalize();
    /// Returns a normalized copy of this vector.
    float2 Normalized() const;

    /// Scales this vector so that its new length is as given. This is effectively the same as normalizing the
    /// vector first and then multiplying by newLength.
    float ScaleToLength(float newLength);

    /// Tests if the length of this vector is one, up to the given epsilon.
    bool IsNormalized(float epsilonSq = 1e-6f) const;

    /// Tests if this is the null vector, up to the given epsilon.
    bool IsZero(float epsilonSq = 1e-6f) const;

    /// Tests if this vector contains valid finite elements.
    bool IsFinite() const;

    /// Tests if two vectors are perpendicular to each other.
    bool IsPerpendicular(const float2 &other, float epsilon = 1e-3f) const;

    bool Equals(const float2 &rhs, float epsilon = 1e-3f) const;
    bool Equals(float x, float y, float epsilon = 1e-3f) const;

    /// Returns "(x, y)" .
    std::string ToString() const;

    /// Returns x + y .
    float SumOfElements() const;
    /// Returns x * y .
    float ProductOfElements() const;
    /// Returns (x+y)/2 .
    float AverageOfElements() const;
    /// Returns min(x, y).
    float MinElement() const;
    /// Returns the index that has the smallest value in this vector.
    int MinElementIndex() const;
    /// Returns max(x, y).
    float MaxElement() const;
    /// Returns the index that has the smallest value in this vector.
    int MaxElementIndex() const;
    /// Takes element-wise absolute value of this vector.
    float2 Abs() const;
    /// Returns an element-wise minimum of this and the vector (ceil, ceil).
    float2 Min(float ceil) const;
    /// Returns an element-wise minimum of this and the given vector.
    float2 Min(const float2 &ceil) const;
    /// Returns an element-wise maximum of this and the vector (floor, floor).
    float2 Max(float floor) const;
    /// Returns an element-wise maximum of this and the given vector.
    float2 Max(const float2 &floor) const;
    /// Returns a copy of this vector that has floor <= this[i] <= ceil for each element.
    float2 Clamp(float floor, float ceil) const;
    /// Limits each element of this vector between the corresponding elements in floor and ceil.
    float2 Clamp(const float2 &floor, const float2 &ceil) const;
    /// Limits each element of this vector in the range [0, 1].
    float2 Clamp01() const;

    /// Computes the distance between this and the given float2.
    float Distance(const float2 &rhs) const;
    /// Computes the squared distance between this and the given float2.
    float DistanceSq(const float2 &rhs) const;
    /// Computes the dot product of this and the given float2.
    float Dot(const float2 &rhs) const;
    /// Computes the perp-dot product of this and the given float2 in the order this^perp <dot> rhs.
    float PerpDot(const float2 &rhs) const;

    /// Returns this vector reflected about a plane with the given normal. By convention, both this and the reflected 
    /// vector point away from the plane with the given normal.
    float2 Reflect(const float2 &normal) const;

    float2 Refract(const float2 &normal, float negativeSideRefractionIndex, float positiveSideRefractionIndex) const;

    /// Projects this vector onto the given unnormalized direction vector.
    float2 ProjectTo(const float2 &direction) const;

    /// Projects this vector onto the given normalized direction vector.
    /// @param direction The vector to project onto. This vector must be normalized.
    float2 ProjectToNorm(const float2 &direction) const;

    /// Returns the angle between this vector and the specified vector, in radians.
    /// @note This function takes into account that this vector or the other vector can be unnormalized, and normalizes the computations.
    float AngleBetween(const float2 &other) const;

    /// Returns the angle between this vector and the specified normalized vector, in radians.
    /// @note This vector must be normalized to call this function.
    float AngleBetweenNorm(const float2 &normalizedVector) const;

    /// Breaks this vector down into parallel and perpendicular components with respect to the given direction.
    void Decompose(const float2 &direction, float2 &outParallel, float2 &outPerpendicular) const;

    /// Linearly interpolates between this and the vector b.
    /// @param t The interpolation weight, in the range [0, 1].
    /// Lerp(b, 0) returns this vector, Lerp(b, 1) returns the vector b.
    /// Lerp(b, 0.5) returns the vector half-way in between the two vectors, and so on.
    float2 Lerp(const float2 &b, float t) const;

    /// Makes the given vectors linearly independent.
    static void Orthogonalize(const float2 &a, float2 &b);

    /// Makes the given vectors linearly independent and normalized in length.
    static void Orthonormalize(float2 &a, float2 &b);

    /// Generates a new float2 by filling its entries by the given scalar .
    static float2 FromScalar(float scalar);

    /// Fills each entry of this float2 by the given scalar .
    void SetFromScalar(float scalar);

    void Set(float x, float y);

    /// Rotates this vector 90 degrees clock-wise [in place].
    /// This is in a coordinate system on a plane where +x extends to the right, and +y extends upwards.
    void Rotate90CW();
    
    /// Rotates this vector 90 degrees counterclock-wise .
    /// This is in a coordinate system on a plane where +x extends to the right, and +y extends upwards.
    void Rotate90CCW();

    /// Returns true if the triangle a->b->c is oriented counter-clockwise, when viewed in the XY-plane
    /// where x spans to the right and y spans up.
    /// Another way to think of this is that this function returns true, if the point C lies to the left
    /// of the directed line AB.
    static bool OrientedCCW(const float2 &a, const float2 &b, const float2 &c);

    float2 operator -() const;

    float2 operator +(const float2 &rhs) const;
    float2 operator -(const float2 &rhs) const;
//    float2 operator *(const float2 &rhs) const;
    float2 operator *(float scalar) const;
//    float2 operator /(const float2 &rhs) const;
    float2 operator /(float scalar) const;

    float2 &operator +=(const float2 &rhs);
    float2 &operator -=(const float2 &rhs);
//    float2 &operator *=(const float2 &rhs);
    float2 &operator *=(float scalar);
//    float2 &operator /=(const float2 &rhs);
    float2 &operator /=(float scalar);

    float2 Add(const float2 &rhs) const { return *this + rhs; }
    float2 Sub(const float2 &rhs) const { return *this - rhs; }
    float2 Mul(float rhs) const { return *this * rhs; }
    float2 Div(float rhs) const { return *this / rhs; }
    float2 Neg() const { return -*this; }

    /// Specifies a compile-time constant float2 with value (0, 0).
    static const float2 zero;
    /// Specifies a compile-time constant float2 with value (1, 1).
    static const float2 one;
    /// Specifies a compile-time constant float2 with value (1, 0).
    static const float2 unitX;
    /// Specifies a compile-time constant float2 with value (0, 1).
    static const float2 unitY;

#ifdef OGRE_INTEROP
    float2(const Ogre::Vector2 &other) { x = other.x; y = other.y; }
    operator Ogre::Vector2() const { return Ogre::Vector3(x, y); }
#endif
#ifdef QT_INTEROP
    float2(const QVector2D &other) { x = other.x(); y = other.y(); }
    operator QVector2D() const { return QVector2D(x, y); }
    operator QString() const { return "(" + QString::number(x) + "," + QString::number(y) + ")"; }
    QString toString() const { return (QString)*this; }
#endif
};

/// Prints this float2 to the given stream.
std::ostream &operator <<(std::ostream &out, const float2 &rhs);

float2 operator *(float scalar, const float2 &rhs);
//float2 operator /(float scalar, const float2 &rhs); 

inline float Dot(const float2 &a, const float2 &b) { return a.Dot(b); }
inline float2 Min(const float2 &a, const float2 &b) { return a.Min(b); }
inline float2 Max(const float2 &a, const float2 &b) { return a.Max(b); }
inline float2 Clamp(const float2 &a, float floor, float ceil) { return a.Clamp(floor, ceil); }
inline float2 Clamp(const float2 &a, const float2 &floor, const float2 &ceil) { return a.Clamp(floor, ceil); }
inline float2 Clamp01(const float2 &a) { return a.Clamp01(); }
inline float2 Lerp(const float2 &a, const float2 &b, float t) { return a.Lerp(b, t); }

#ifdef QT_INTEROP
Q_DECLARE_METATYPE(float2)
Q_DECLARE_METATYPE(float2*)
#endif
