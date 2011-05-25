/** @file Quat.h
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 

    @brief Quaternions represent rotations and orientations of 3D objects.
*/
#pragma once

#include <string>
#include "MathFwd.h"

#ifdef QT_INTEROP
#include <QQuaternion>
#endif

#ifdef IRRLICHT_INTEROP
#include "Quaternion.h"
#endif

#ifdef OGRE_INTEROP
#include <OgreQuaternion.h>
#endif

/// Represents a rotation or an orientation of a 3D object.
class Quat
{
public:
    float x; ///< The factor of i.
    float y; ///< The factor of j.
    float z; ///< The factor of k.
    float w; ///< The scalar part. Sometimes also referred to as 'r'.

    /// @note The default ctor does not initialize any member values.
    Quat() {}

    /// Constructs a quaternion from the given data buffer.
    /// @param data An array of four floats to use for the quaternion, in the order 'x, y, z, w'. (== 'i, j, k, r')
    /// @note The input data is not normalized after construction, this has to be done manually.
    explicit Quat(const float *data);

    explicit Quat(const float3x3 &rotationMatrix);
    explicit Quat(const float3x4 &rotationMatrix);
    explicit Quat(const float4x4 &rotationMatrix);

    /// @param x The factor of i.
    /// @param y The factor of j.
    /// @param z The factor of k.
    /// @param w The scalar factor (or 'w').
    /// @note The input data is not normalized after construction, this has to be done manually.
    Quat(float x, float y, float z, float w);

    Quat(const float3 &rotationAxis, float rotationAngle);

    /// Returns the local +X axis in the post-transformed coordinate space. This is the same as transforming the vector (1,0,0) by this quaternion.
    float3 WorldX() const;
    /// Returns the local +Y axis in the post-transformed coordinate space. This is the same as transforming the vector (0,1,0) by this quaternion.
    float3 WorldY() const;
    /// Returns the local +Z axis in the post-transformed coordinate space. This is the same as transforming the vector (0,0,1) by this quaternion.
    float3 WorldZ() const;

    /// Returns the axis of rotation for this quaternion.
    float3 Axis() const;

    /// Returns the angle of rotation for this quaternion, in radians.
    float Angle() const;

    /// Computes the dot product of this and the given quaternion.
    /// Dot product is commutative.
    float Dot(const Quat &rhs) const;

    float LengthSq() const;

    float Length() const;

    /// Normalizes this quaternion in-place.
    /// Returns the old length of this quaternion, or 0 if normalization failed.
    float Normalize();

    /// Returns a normalized copy of this quaternion.
    Quat Normalized() const;

    /// Returns true if the length of this quaternion is one.
    bool IsNormalized(float epsilon = 1e-3f) const;

    bool IsInvertible(float epsilon = 1e-3f) const;

    /// Returns true if the entries of this quaternion are all finite.
    bool IsFinite() const;

    /// @return A pointer to the first element (x). The data is contiguous in memory.
    /// ptr[0] gives x, ptr[1] is y, ptr[2] is z and ptr[3] is w.
    float *ptr();
    const float *ptr() const;

    /// Inverses this quaternion in-place.
    /// @note For optimization purposes, this function assumes that the quaternion is unitary, in which
    ///       case the inverse of the quaternion is simply just the same as its conjugate. This function
    ///       does not detect whether the operation succeeded or failed.
    void Inverse();

    /// Returns an inverted copy of this quaternion.
    Quat Inverted() const;

    /// Inverses this quaternion in-place.
    /// Call this function when the quaternion is not known beforehand to be normalized. This function
    /// computes the inverse proper, and normalizes the result.
    /// @note Because of the normalization, it does not necessarily hold that q * q.InverseAndNormalize() == id.
    /// @return Returns the old length of this quaternion (not the old length of the inverse quaternion).
    float InverseAndNormalize();

    /// Computes the conjugate of this quaternion in-place.
    void Conjugate();

    /// Returns a conjugated copy of this quaternion.
    Quat Conjugated() const;

    /// Rotates the given vector by this quaternion.
    float3 Transform(float x, float y, float z) const;
    float3 Transform(const float3 &vec) const;

    /// Rotates the given vector by this quaternion. The w component of the vector is ignored.
    float4 Transform(const float4 &vec) const;

    Quat Lerp(const Quat &target, float t) const;
    Quat Slerp(const Quat &target, float t) const;

    /// Returns the angle between this and the target orientation (the shortest route).
    float AngleBetween(const Quat &target) const;
    /// Returns the axis of rotation to get from this orientation to target orientation (the shortest route).
    float3 AxisFromTo(const Quat &target) const;

    void ToAxisAngle(float3 &rotationAxis, float &rotationAngle) const;
    void SetFromAxisAngle(const float3 &rotationAxis, float rotationAngle);

    /// Sets this quaternion to represent the same rotation as the given matrix.
    void Set(const float3x3 &matrix);
    void Set(const float3x4 &matrix);
    void Set(const float4x4 &matrix);

    /// Applies a rotation to this quaternion so that the local (pre-transformed) axis specified by localForward 
    /// looks towards targetDirection. Then tries to orient the localUp axis to point towards the
    /// worldUp axis.
    void LookAt(const float3 &localForward, const float3 &targetDirection, const float3 &localUp, const float3 &worldUp);

    /// Creates a new quaternion that rotates about the positive X axis by the given angle.
    static Quat RotateX(float angle);
    /// Creates a new quaternion that rotates about the positive Y axis by the given angle.
    static Quat RotateY(float angle);
    /// Creates a new quaternion that rotates about the positive Z axis by the given angle.
    static Quat RotateZ(float angle);

    /// Creates a new Quat that rotates about the given axis by the given angle.
    static Quat RotateAxisAngle(const float3 &axisDirection, float angle);

    /// Creates a new quaternion that rotates sourceDirection vector (in world space) to coincide with the 
    /// targetDirection vector (in world space).
    /// Rotation is performed around the origin.
    /// \note There are infinite such rotations - this function returns the rotation that has the shortest angle
    /// (when decomposed to axis-angle notation).
    static Quat RotateFromTo(const float3 &sourceDirection, const float3 &targetDirection);

    /// Creates a new quaternion that
    /// 1. rotates sourceDirection vector to coincide with the targetDirection vector, and then 
    /// 2. rotates sourceDirection2 (which was transformed by 1.) to targetDirection2, but keeping the constraint that 
    ///    sourceDirection must look at targetDirection.
    static Quat RotateFromTo(const float3 &sourceDirection, const float3 &targetDirection,
        const float3 &sourceDirection2, const float3 &targetDirection2);

    /// Creates a new Quat from the given sequence of Euler rotation angles.
    /** The FromEulerABC function returns a matrix M = A(a) * B(b) * C(c). Rotation
        C is applied first, followed by B and then A. [indexTitle: FromEuler***] */
    static Quat FromEulerXYX(float x2, float y, float x);
    static Quat FromEulerXZX(float x2, float z, float x); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerYXY(float y2, float x, float y); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerYZY(float y2, float z, float y); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerZXZ(float z2, float x, float z); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerZYZ(float z2, float y, float z); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerXYZ(float x, float y, float z); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerXZY(float x, float z, float y); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerYXZ(float y, float x, float z); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerYZX(float y, float z, float x); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerZXY(float z, float x, float y); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static Quat FromEulerZYX(float z, float y, float x); ///< [similarOverload: FromEulerXYX] [hideIndex]

    /// Extracts the rotation part of this quaternion into Euler rotation angles.
    float3 ToEulerXYX() const;
    float3 ToEulerXZX() const;
    float3 ToEulerYXY() const;
    float3 ToEulerYZY() const;
    float3 ToEulerZXZ() const;
    float3 ToEulerZYZ() const;
    float3 ToEulerXYZ() const;
    float3 ToEulerXZY() const;
    float3 ToEulerYXZ() const;
    float3 ToEulerYZX() const;
    float3 ToEulerZXY() const;
    float3 ToEulerZYX() const;

    float3x3 ToFloat3x3() const;
    float3x4 ToFloat3x4() const;
    float4x4 ToFloat4x4() const;

    std::string ToString();

    /// Multiplies two quaternions together.
    /// The product q1 * q2 returns a quaternion that concatenates the two orientation rotations. The rotation
    /// q2 is applied first before q1.
    Quat operator *(const Quat &rhs) const;

    /// The identity quaternion performs no rotation when applied to a vector.
    /// For quaternions, the identity has the value r = 1, i,j,k = 0.
    static const Quat identity;

    /// Divides a quaternion by another. Division "a / b" results in a quaternion that rotates the orientation b to coincide with the orientation a.
    Quat operator /(const Quat &rhs) const;

#ifdef OGRE_INTEROP
    Quat(const Ogre::Quaternion &other) { w = other.w; x = other.x; y = other.y; z = other.z; }
    operator Ogre::Quaternion() const { return Ogre::Quaternion(w, x, y, z); }
#endif
#ifdef IRRLICHT_INTEROP
    Quat(const Quaternion &other) { w = other.w; x = other.x; y = other.y; z = other.z; }
    operator Quaternion() const { return Quaternion(x, y, z, w); }
#endif
#ifdef QT_INTEROP
    Quat(const QQuaternion &other) { w = other.scalar(); x = other.x(); y = other.y(); z = other.z(); }
    operator QQuaternion() const { return QQuaternion(w, x, y, z); }
    operator QString() const { return "(" + QString::number(x) + "," + QString::number(y) + "," + QString::number(z) + "," + QString::number(w) + ")"; }
#endif
#ifdef BULLET_INTEROP
    Quat(const btQuaternion &other) { w = other.w(); x = other.x(); y = other.y(); z = other.z(); }
    operator btQuaternion() const { return btQuaternion(x, y, z, w); }
#endif

private: // Hide the unsafe operations from the user, so that he doesn't accidentally invoke an unintended operation.

    /// Multiplies a quaternion by a scalar.
    /// @note Technically, multiplication by scalar would not affect the rotation this quaternion represents, but since
    /// Quat uses conjugation to compute the inverse (to optimize), an unnormalized quaternion will not produce a proper rotation transform.
    /// @important Multiplication by a scalar does not "accumulate" rotations, e.g. "quat * 5.f" will not produce a quaternion that would rotate
    ///            "5 times more".
    Quat operator *(float scalar) const;

    Quat operator /(float scalar) const;

    /// Adds two quaternions.
    /// @important Adding two quaternions does not concatenate the two rotation operations. Use quaternion multiplication to achieve that.
    Quat operator +(const Quat &rhs) const;

    Quat operator -(const Quat &rhs) const;

    /// Negates the quaternion.
    /// @important Negating a quaternion will not produce the inverse rotation. Call Quat::Inverse() to generate the inverse rotation.
    Quat operator -() const;
};

Quat Lerp(const Quat &a, const Quat &b, float t);
Quat Slerp(const Quat &a, const Quat &b, float t);
