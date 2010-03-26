// Based on Irrlicht Engine implementation.
// This file is under following license:
//
// The Irrlicht Engine License
//
// Copyright © 2002-2005 Nikolaus Gebhardt
//
// This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions: 
//
// The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// This notice may not be removed or altered from any source distribution.

#ifndef incl_Core_Vector3D_h
#define incl_Core_Vector3D_h

#include "CoreMath.h"
#include "CoreStringUtils.h"

//! 3d vector template class with lots of operators and methods.
/** The Vector3D class is used in Irrlicht for three main purposes: 
	1) As a direction vector (most of the methods assume this).
	2) As a position in 3d space (which is synonymous with a direction vector from the origin to this position).
	3) To hold three Euler rotations, where x is pitch, y is yaw and z is roll.
*/
template <class T>
class Vector3D
{
public:
	//! Default constructor (null vector).
	Vector3D() : x(0), y(0), z(0) {}
	//! Constructor with three different values
	Vector3D(T nx, T ny, T nz) : x(nx), y(ny), z(nz) {}
	//! Constructor with the same value for all elements
	explicit Vector3D(T n) : x(n), y(n), z(n) {}
	//! Copy constructor
	Vector3D(const Vector3D<T>& other) : x(other.x), y(other.y), z(other.z) {}

	// operators

	Vector3D<T> operator-() const { return Vector3D<T>(-x, -y, -z); }

	Vector3D<T>& operator=(const Vector3D<T>& other) { x = other.x; y = other.y; z = other.z; return *this; }

	Vector3D<T> operator+(const Vector3D<T>& other) const { return Vector3D<T>(x + other.x, y + other.y, z + other.z); }
	Vector3D<T>& operator+=(const Vector3D<T>& other) { x+=other.x; y+=other.y; z+=other.z; return *this; }
	Vector3D<T> operator+(const T val) const { return Vector3D<T>(x + val, y + val, z + val); }
	Vector3D<T>& operator+=(const T val) { x+=val; y+=val; z+=val; return *this; }

	Vector3D<T> operator-(const Vector3D<T>& other) const { return Vector3D<T>(x - other.x, y - other.y, z - other.z); }
	Vector3D<T>& operator-=(const Vector3D<T>& other) { x-=other.x; y-=other.y; z-=other.z; return *this; }
	Vector3D<T> operator-(const T val) const { return Vector3D<T>(x - val, y - val, z - val); }
	Vector3D<T>& operator-=(const T val) { x-=val; y-=val; z-=val; return *this; }

	Vector3D<T> operator*(const Vector3D<T>& other) const { return Vector3D<T>(x * other.x, y * other.y, z * other.z); }
	Vector3D<T>& operator*=(const Vector3D<T>& other) { x*=other.x; y*=other.y; z*=other.z; return *this; }
	Vector3D<T> operator*(const T v) const { return Vector3D<T>(x * v, y * v, z * v); }
	Vector3D<T>& operator*=(const T v) { x*=v; y*=v; z*=v; return *this; }

	Vector3D<T> operator/(const Vector3D<T>& other) const { return Vector3D<T>(x / other.x, y / other.y, z / other.z); }
	Vector3D<T>& operator/=(const Vector3D<T>& other) { x/=other.x; y/=other.y; z/=other.z; return *this; }
	Vector3D<T> operator/(const T v) const { T i=(T)1.0/v; return Vector3D<T>(x * i, y * i, z * i); }
	Vector3D<T>& operator/=(const T v) { T i=(T)1.0/v; x*=i; y*=i; z*=i; return *this; }

	bool operator<=(const Vector3D<T>&other) const { return x<=other.x && y<=other.y && z<=other.z;}
	bool operator>=(const Vector3D<T>&other) const { return x>=other.x && y>=other.y && z>=other.z;}
	bool operator<(const Vector3D<T>&other) const { return x<other.x && y<other.y && z<other.z;}
	bool operator>(const Vector3D<T>&other) const { return x>other.x && y>other.y && z>other.z;}

	//! use weak float compare
	bool operator==(const Vector3D<T>& other) const
	{
		return this->equals(other);
	}

	bool operator!=(const Vector3D<T>& other) const
	{
		return !this->equals(other);
	}

	// functions

	//! returns if this vector equals the other one, taking floating point rounding errors into account
	bool equals(const Vector3D<T>& other, const T tolerance = (T)ROUNDING_ERROR_32 ) const
	{
		return ::equals(x, other.x, tolerance) &&
			::equals(y, other.y, tolerance) &&
			::equals(z, other.z, tolerance);
	}

	Vector3D<T>& set(const T nx, const T ny, const T nz) {x=nx; y=ny; z=nz; return *this;}
	Vector3D<T>& set(const Vector3D<T>& p) {x=p.x; y=p.y; z=p.z;return *this;}

	//! Get length of the vector.
	T getLength() const { return (T) sqrt((f64)(x*x + y*y + z*z)); }

	//! Get squared length of the vector.
	/** This is useful because it is much faster than getLength().
	\return Squared length of the vector. */
	T getLengthSQ() const { return x*x + y*y + z*z; }

	//! Get the dot product with another vector.
	T dotProduct(const Vector3D<T>& other) const
	{
		return x*other.x + y*other.y + z*other.z;
	}

	//! Get distance from another point.
	/** Here, the vector is interpreted as point in 3 dimensional space. */
	T getDistanceFrom(const Vector3D<T>& other) const
	{
		return Vector3D<T>(x - other.x, y - other.y, z - other.z).getLength();
	}

	//! Returns squared distance from another point.
	/** Here, the vector is interpreted as point in 3 dimensional space. */
	T getDistanceFromSQ(const Vector3D<T>& other) const
	{
		return Vector3D<T>(x - other.x, y - other.y, z - other.z).getLengthSQ();
	}

	//! Calculates the cross product with another vector.
	/** \param p Vector to multiply with.
	\return Crossproduct of this vector with p. */
	Vector3D<T> crossProduct(const Vector3D<T>& p) const
	{
		return Vector3D<T>(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x);
	}

	//! Returns if this vector interpreted as a point is on a line between two other points.
	/** It is assumed that the point is on the line.
	\param begin Beginning vector to compare between.
	\param end Ending vector to compare between.
	\return True if this vector is between begin and end, false if not. */
	bool isBetweenPoints(const Vector3D<T>& begin, const Vector3D<T>& end) const
	{
		const T f = (end - begin).getLengthSQ();
		return getDistanceFromSQ(begin) <= f &&
			getDistanceFromSQ(end) <= f;
	}

	//! Normalizes the vector.
	/** In case of the 0 vector the result is still 0, otherwise
	the length of the vector will be 1.
	\return Reference to this vector after normalization. */
	Vector3D<T>& normalize()
	{
		f32 length = (f32)(x*x + y*y + z*z);
		if (::equals(length, 0.f))
			return *this;
		length = reciprocal_squareroot ( (f32)length );
		x = (T)(x * length);
		y = (T)(y * length);
		z = (T)(z * length);
		return *this;
	}

	//! Sets the length of the vector to a new value
	Vector3D<T>& setLength(T newlength)
	{
		normalize();
		return (*this *= newlength);
	}

	//! Inverts the vector.
	Vector3D<T>& invert()
	{
		x *= -1.0f;
		y *= -1.0f;
		z *= -1.0f;
		return *this;
	}

	//! Rotates the vector by a specified number of degrees around the y axis and the specified center.
	/** \param degrees Number of degrees to rotate around the y axis.
	\param center The center of the rotation. */
	void rotateXZBy(f64 degrees, const Vector3D<T>& center=Vector3D<T>())
	{
		degrees *= DEGTORAD64;
		f64 cs = cos(degrees);
		f64 sn = sin(degrees);
		x -= center.x;
		z -= center.z;
		set((T)(x*cs - z*sn), y, (T)(x*sn + z*cs));
		x += center.x;
		z += center.z;
	}

	//! Rotates the vector by a specified number of degrees around the z axis and the specified center.
	/** \param degrees: Number of degrees to rotate around the z axis.
	\param center: The center of the rotation. */
	void rotateXYBy(f64 degrees, const Vector3D<T>& center=Vector3D<T>())
	{
		degrees *= DEGTORAD64;
		f64 cs = cos(degrees);
		f64 sn = sin(degrees);
		x -= center.x;
		y -= center.y;
		set((T)(x*cs - y*sn), (T)(x*sn + y*cs), z);
		x += center.x;
		y += center.y;
	}

	//! Rotates the vector by a specified number of degrees around the x axis and the specified center.
	/** \param degrees: Number of degrees to rotate around the x axis.
	\param center: The center of the rotation. */
	void rotateYZBy(f64 degrees, const Vector3D<T>& center=Vector3D<T>())
	{
		degrees *= DEGTORAD64;
		f64 cs = cos(degrees);
		f64 sn = sin(degrees);
		z -= center.z;
		y -= center.y;
		set(x, (T)(y*cs - z*sn), (T)(y*sn + z*cs));
		z += center.z;
		y += center.y;
	}

	//! Creates an interpolated vector between this vector and another vector.
	/** \param other The other vector to interpolate with.
	\param d Interpolation value between 0.0f (all the other vector) and 1.0f (all this vector).
	Note that this is the opposite direction of interpolation to getInterpolated_quadratic()
	\return An interpolated vector.  This vector is not modified. */
	Vector3D<T> getInterpolated(const Vector3D<T>& other, f64 d) const
	{
		const f64 inv = 1.0 - d;
		return Vector3D<T>((T)(other.x*inv + x*d), (T)(other.y*inv + y*d), (T)(other.z*inv + z*d));
	}

	//! Creates a quadratically interpolated vector between this and two other vectors.
	/** \param v2 Second vector to interpolate with.
	\param v3 Third vector to interpolate with (maximum at 1.0f)
	\param d Interpolation value between 0.0f (all this vector) and 1.0f (all the 3rd vector).
	Note that this is the opposite direction of interpolation to getInterpolated() and interpolate()
	\return An interpolated vector. This vector is not modified. */
	Vector3D<T> getInterpolated_quadratic(const Vector3D<T>& v2, const Vector3D<T>& v3, f64 d) const
	{
		// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
		const f64 inv = (T) 1.0 - d;
		const f64 mul0 = inv * inv;
		const f64 mul1 = (T) 2.0 * d * inv;
		const f64 mul2 = d * d;

		return Vector3D<T> ((T)(x * mul0 + v2.x * mul1 + v3.x * mul2),
				(T)(y * mul0 + v2.y * mul1 + v3.y * mul2),
				(T)(z * mul0 + v2.z * mul1 + v3.z * mul2));
	}

	//! Sets this vector to the linearly interpolated vector between a and b.
	/** \param a first vector to interpolate with, maximum at 1.0f
	\param b second vector to interpolate with, maximum at 0.0f
	\param d Interpolation value between 0.0f (all vector b) and 1.0f (all vector a)
	Note that this is the opposite direction of interpolation to getInterpolated_quadratic()
	*/
	Vector3D<T>& interpolate(const Vector3D<T>& a, const Vector3D<T>& b, f64 d)
	{
		x = (T)((f64)b.x + ( ( a.x - b.x ) * d ));
		y = (T)((f64)b.y + ( ( a.y - b.y ) * d ));
		z = (T)((f64)b.z + ( ( a.z - b.z ) * d ));
		return *this;
	}


	//! Get the rotations that would make a (0,0,1) direction vector point in the same direction as this direction vector.
	/** Thanks to Arras on the Irrlicht forums for this method.  This utility method is very useful for
	orienting scene nodes towards specific targets.  For example, if this vector represents the difference
	between two scene nodes, then applying the result of getHorizontalAngle() to one scene node will point
	it at the other one.
	Example code:
	// Where target and seeker are of type ISceneNode*
	const vector3df toTarget(target->getAbsolutePosition() - seeker->getAbsolutePosition());
	const vector3df requiredRotation = toTarget.getHorizontalAngle();
	seeker->setRotation(requiredRotation); 

	\return A rotation vector containing the x (pitch) and y (raw) rotations (in degrees) that when applied to a 
	+z (e.g. 0, 0, 1) direction vector would make it point in the same direction as this vector. The z (roll) rotation 
	is always 0, since two Euler rotations are sufficient to point in any given direction. */
	Vector3D<T> getHorizontalAngle() const
	{
		Vector3D<T> angle;

		angle.y = (T)(atan2(x, z) * RADTODEG64);

		if (angle.y < 0.0f)
			angle.y += 360.0f;
		if (angle.y >= 360.0f)
			angle.y -= 360.0f;

		const f64 z1 = sqrt(x*x + z*z);

		angle.x = (T)(atan2(z1, (f64)y) * RADTODEG64 - 90.0);

		if (angle.x < 0.0f)
			angle.x += 360.0f;
		if (angle.x >= 360.0f)
			angle.x -= 360.0f;

		return angle;
	}

	//! Builds a direction vector from (this) rotation vector.
	/** This vector is assumed to be a rotation vector composed of 3 Euler angle rotations, in degrees.
	The implementation performs the same calculations as using a matrix to do the rotation.

	\param[in] forwards  The direction representing "forwards" which will be rotated by this vector. 
	If you do not provide a direction, then the +z axis (0, 0, 1) will be assumed to be forwards.
	\return A direction vector calculated by rotating the forwards direction by the 3 Euler angles 
	(in degrees) represented by this vector. */
	Vector3D<T> rotationToDirection(const Vector3D<T> & forwards = Vector3D<T>(0, 0, 1)) const
	{
		const f64 cr = cos( DEGTORAD64 * x );
		const f64 sr = sin( DEGTORAD64 * x );
		const f64 cp = cos( DEGTORAD64 * y );
		const f64 sp = sin( DEGTORAD64 * y );
		const f64 cy = cos( DEGTORAD64 * z );
		const f64 sy = sin( DEGTORAD64 * z );

		const f64 srsp = sr*sp;
		const f64 crsp = cr*sp;

		const f64 pseudoMatrix[] = {
			( cp*cy ), ( cp*sy ), ( -sp ),
			( srsp*cy-cr*sy ), ( srsp*sy+cr*cy ), ( sr*cp ),
			( crsp*cy+sr*sy ), ( crsp*sy-sr*cy ), ( cr*cp )};

		return Vector3D<T>(
			(T)(forwards.x * pseudoMatrix[0] +
				forwards.y * pseudoMatrix[3] +
				forwards.z * pseudoMatrix[6]),
			(T)(forwards.x * pseudoMatrix[1] +
				forwards.y * pseudoMatrix[4] +
				forwards.z * pseudoMatrix[7]),
			(T)(forwards.x * pseudoMatrix[2] +
				forwards.y * pseudoMatrix[5] +
				forwards.z * pseudoMatrix[8]));
	}

	//! Fills an array of 4 values with the vector data (usually floats).
	/** Useful for setting in shader constants for example. The fourth value
	will always be 0. */
	void getAs4Values(T* array) const
	{
		array[0] = x;
		array[1] = y;
		array[2] = z;
		array[3] = 0;
	}

    friend std::ostream& operator << ( std::ostream& out, const Vector3D<T>& v )
    {
        out << "Vector3D(" << v.x << ", " << v.y << ", " << v.z << ")";
        return out;
    }

    friend std::istream& operator >> ( std::istream& in, Vector3D<T>& v )
    {
        std::string str;
        std::getline(in, str);

        size_t pos = str.find_last_of('(');
        if (pos != std::string::npos)
        {
            size_t val = str.find(',', pos);
            if (val != std::string::npos)
            {
                T xx = ParseString<T>(str.substr(pos + 1, val - pos - 1));

                pos = val;
                val = str.find(',', pos + 1);
                if (val != std::string::npos)
                {
                    T yy = ParseString<T>(str.substr(pos + 2, val - pos - 2));
                    
                    pos = val;
                    val = str.find(')', pos + 1);
                    if (val != std::string::npos)
                    {
                        T zz = ParseString<T>(str.substr(pos + 2, val - pos - 2));
                        v.x = xx;
                        v.y = yy;
                        v.z = zz;
                    }
                }
            }
        }

        return in;
    }

    static const Vector3D<f32> ZERO;
    static const Vector3D<f32> UNIT_SCALE_X;
    static const Vector3D<f32> UNIT_X;
    static const Vector3D<f32> UNIT_Y;
    static const Vector3D<f32> UNIT_Z;
    static const Vector3D<f32> NEGATIVE_UNIT_X;
    static const Vector3D<f32> NEGATIVE_UNIT_Y;
    static const Vector3D<f32> NEGATIVE_UNIT_Z;

	//! x coordinate of the vector
	T x;
	//! y coordinate of the vector
	T y;
	//! z coordinate of the vector
	T z;
};

//   template class<T>
//   Vector3D<f32> Vector3D<f32>::ZERO(0, 0, 0);


//! Typedef for a f32 3d vector.
typedef Vector3D<f32> Vector3df;
typedef Vector3df Vector3Df;
//typedef Vector3df Vector3;

//! Typedef for an integer 3d vector.
typedef Vector3D<s32> Vector3di;

//! Function multiplying a scalar and a vector component-wise.
template<class S, class T>
Vector3D<T> operator*(const S scalar, const Vector3D<T>& vector) { return vector*scalar; }

template <class T>
const Vector3df Vector3D<T>::ZERO;

template <class T>
const Vector3df Vector3D<T>::UNIT_SCALE_X(1, 1, 1);

template <class T>
const Vector3df Vector3D<T>::UNIT_X(1, 0, 0);

template <class T>
const Vector3df Vector3D<T>::UNIT_Y(0, 1, 0);

template <class T>
const Vector3df Vector3D<T>::UNIT_Z(0, 0, 1);

template <class T>
const Vector3df Vector3D<T>::NEGATIVE_UNIT_X(-1, 0, 0);

template <class T>
const Vector3df Vector3D<T>::NEGATIVE_UNIT_Y(0, -1, 0);

template <class T>
const Vector3df Vector3D<T>::NEGATIVE_UNIT_Z(0, 0, -1);



#endif

