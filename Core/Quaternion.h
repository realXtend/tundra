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

#ifndef incl_Core_Quaternion_h
#define incl_Core_Quaternion_h

#include "CoreMath.h"
#include "Vector3D.h"
#include "Matrix4.h"

#include <QMetaType>

//! Quaternion class for representing rotations.
/** It provides cheap combinations and avoids gimbal locks.
Also useful for interpolations. */
class Quaternion
{
	public:

		//! Default Constructor
		Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

		//! Constructor
		Quaternion(f32 xx, f32 yy, f32 zz, f32 ww) : x(xx), y(yy), z(zz), w(ww) { }

		//! Constructor which converts euler angles (radians) to a Quaternion
		Quaternion(f32 x, f32 y, f32 z);

		//! Constructor which converts euler angles (radians) to a Quaternion
		Quaternion(const Vector3df& vec);

		//! Constructor which converts a matrix to a Quaternion
		Quaternion(const Matrix4& mat);

		//! Constructor which creates quaternion from rotation angle and rotation axis.
		/** Axis must be unit length.
		The Quaternion representing the rotation is
		q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k).
		\param angle Rotation Angle in radians.
		\param axis Rotation axis. */
		Quaternion(f32 angle, const Vector3df& axis)
		{
		    fromAngleAxis(angle, axis);
		}

		//! Equalilty operator
		bool operator==(const Quaternion& other) const;

		//! Inequality operator
		bool operator!=(const Quaternion& other) const;

        bool equals(const Quaternion other, const f32 tolerance = ROUNDING_ERROR_32 ) const
		{
			return ::equals(x, other.x, tolerance) &&
				::equals(y, other.y, tolerance) &&
				::equals(z, other.z, tolerance) &&
                ::equals(w, other.w, tolerance);
		}

		//! Assignment operator
		inline Quaternion& operator=(const Quaternion& other);

		//! Matrix assignment operator
		inline Quaternion& operator=(const Matrix4& other);

		//! Add operator
		Quaternion operator+(const Quaternion& other) const;

		//! Multiplication operator
		Quaternion operator*(const Quaternion& other) const;

		//! Multiplication operator with scalar
		Quaternion operator*(f32 s) const;

		//! Multiplication operator with scalar
		Quaternion& operator*=(f32 s);

		//! Multiplication operator
		Vector3df operator*(const Vector3df& v) const;

		//! Multiplication operator
		Quaternion& operator*=(const Quaternion& other);

		//! Calculates the dot product
		inline f32 dotProduct(const Quaternion& other) const;

		//! Sets new Quaternion
		inline Quaternion& set(f32 xx, f32 yy, f32 zz, f32 ww);

		//! Sets new Quaternion based on euler angles (radians)
		inline Quaternion& set(f32 xx, f32 yy, f32 zz);

		//! Sets new Quaternion based on euler angles (radians)
		inline Quaternion& set(const Vector3df& vec);

		//! Normalizes the Quaternion
		inline Quaternion& normalize();

		//! Creates a matrix from this Quaternion
		Matrix4 getMatrix() const;

		//! Creates a matrix from this Quaternion
		void getMatrix( Matrix4 &dest ) const;

		//! Creates a matrix from this Quaternion
		inline void getMatrix_transposed( Matrix4 &dest ) const;

		//! Inverts this Quaternion
		Quaternion& makeInverse();

		//! Set this Quaternion to the result of the interpolation between two quaternions
		Quaternion& slerp( Quaternion q1, Quaternion q2, f32 interpolate );

		//! Create Quaternion from rotation angle and rotation axis.
		/** Axis must be unit length.
		The Quaternion representing the rotation is
		q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k).
		\param angle Rotation Angle in radians.
		\param axis Rotation axis. */
		Quaternion& fromAngleAxis (f32 angle, const Vector3df& axis);

		//! Fills an angle (radians) around an axis (unit vector)
		void toAngleAxis (f32 &angle, Vector3df& axis) const;

		//! Output this Quaternion to an euler angle (radians)
		void toEuler(Vector3df& euler) const;

		//! Set Quaternion to identity
		Quaternion& makeIdentity();

		//! Set Quaternion to represent a rotation from one vector to another.
		Quaternion& rotationFromTo(const Vector3df& from, const Vector3df& to);

        friend std::ostream& operator << ( std::ostream& out, const Quaternion& q )
        {
            out << "Quaternion(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
            return out;
        }

        static const Quaternion ZERO;
        static const Quaternion IDENTITY;

		//! Quaternion elements.
		f32 x, y, z, w;
};


// Constructor which converts euler angles to a Quaternion
inline Quaternion::Quaternion(f32 x, f32 y, f32 z)
{
	set(x,y,z);
}


// Constructor which converts euler angles to a Quaternion
inline Quaternion::Quaternion(const Vector3df& vec)
{
	set(vec.x,vec.y,vec.z);
}


// Constructor which converts a matrix to a Quaternion
inline Quaternion::Quaternion(const Matrix4& mat)
{
	(*this) = mat;
}


// equal operator
inline bool Quaternion::operator==(const Quaternion& other) const
{
    return equals(other);
}

// equal operator
inline bool Quaternion::operator!=(const Quaternion& other) const
{
    return !equals(other);
}



// assignment operator
inline Quaternion& Quaternion::operator=(const Quaternion& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}


// matrix assignment operator
inline Quaternion& Quaternion::operator=(const Matrix4& m)
{
	const f32 diag = m(0,0) + m(1,1) + m(2,2) + 1;

	if( diag > 0.0f )
	{
		const f32 scale = sqrtf(diag) * 2.0f; // get scale from diagonal

		// TODO: speed this up
		x = ( m(2,1) - m(1,2)) / scale;
		y = ( m(0,2) - m(2,0)) / scale;
		z = ( m(1,0) - m(0,1)) / scale;
		w = 0.25f * scale;
	}
	else
	{
		if ( m(0,0) > m(1,1) && m(0,0) > m(2,2))
		{
			// 1st element of diag is greatest value
			// find scale according to 1st element, and double it
			const f32 scale = sqrtf( 1.0f + m(0,0) - m(1,1) - m(2,2)) * 2.0f;

			// TODO: speed this up
			x = 0.25f * scale;
			y = (m(0,1) + m(1,0)) / scale;
			z = (m(2,0) + m(0,2)) / scale;
			w = (m(2,1) - m(1,2)) / scale;
		}
		else if ( m(1,1) > m(2,2))
		{
			// 2nd element of diag is greatest value
			// find scale according to 2nd element, and double it
			const f32 scale = sqrtf( 1.0f + m(1,1) - m(0,0) - m(2,2)) * 2.0f;

			// TODO: speed this up
			x = (m(0,1) + m(1,0) ) / scale;
			y = 0.25f * scale;
			z = (m(1,2) + m(2,1) ) / scale;
			w = (m(0,2) - m(2,0) ) / scale;
		}
		else
		{
			// 3rd element of diag is greatest value
			// find scale according to 3rd element, and double it
			const f32 scale = sqrtf( 1.0f + m(2,2) - m(0,0) - m(1,1)) * 2.0f;

			// TODO: speed this up
			x = (m(0,2) + m(2,0)) / scale;
			y = (m(1,2) + m(2,1)) / scale;
			z = 0.25f * scale;
			w = (m(1,0) - m(0,1)) / scale;
		}
	}

	return normalize();
}


// multiplication operator
inline Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion tmp;

	tmp.w = (other.w * w) - (other.x * x) - (other.y * y) - (other.z * z);
	tmp.x = (other.w * x) + (other.x * w) + (other.y * z) - (other.z * y);
	tmp.y = (other.w * y) + (other.y * w) + (other.z * x) - (other.x * z);
	tmp.z = (other.w * z) + (other.z * w) + (other.x * y) - (other.y * x);

	return tmp;
}


// multiplication operator
inline Quaternion Quaternion::operator*(f32 s) const
{
	return Quaternion(s*x, s*y, s*z, s*w);
}

// multiplication operator
inline Quaternion& Quaternion::operator*=(f32 s)
{
	x*=s;
	y*=s;
	z*=s;
	w*=s;
	return *this;
}

// multiplication operator
inline Quaternion& Quaternion::operator*=(const Quaternion& other)
{
	return (*this = other * (*this));
}

// add operator
inline Quaternion Quaternion::operator+(const Quaternion& b) const
{
	return Quaternion(x+b.x, y+b.y, z+b.z, w+b.w);
}


// Creates a matrix from this Quaternion
inline Matrix4 Quaternion::getMatrix() const
{
	Matrix4 m;
	getMatrix_transposed(m);
	return m;
}


// Creates a matrix from this Quaternion
inline void Quaternion::getMatrix( Matrix4 &dest ) const
{
	dest[0] = 1.0f - 2.0f*y*y - 2.0f*z*z;
	dest[1] = 2.0f*x*y + 2.0f*z*w;
	dest[2] = 2.0f*x*z - 2.0f*y*w;
	dest[3] = 0.0f;

	dest[4] = 2.0f*x*y - 2.0f*z*w;
	dest[5] = 1.0f - 2.0f*x*x - 2.0f*z*z;
	dest[6] = 2.0f*z*y + 2.0f*x*w;
	dest[7] = 0.0f;

	dest[8] = 2.0f*x*z + 2.0f*y*w;
	dest[9] = 2.0f*z*y - 2.0f*x*w;
	dest[10] = 1.0f - 2.0f*x*x - 2.0f*y*y;
	dest[11] = 0.0f;

	dest[12] = 0.f;
	dest[13] = 0.f;
	dest[14] = 0.f;
	dest[15] = 1.f;
}

// Creates a matrix from this Quaternion
inline void Quaternion::getMatrix_transposed( Matrix4 &dest ) const
{
	dest[0] = 1.0f - 2.0f*y*y - 2.0f*z*z;
	dest[4] = 2.0f*x*y + 2.0f*z*w;
	dest[8] = 2.0f*x*z - 2.0f*y*w;
	dest[12] = 0.0f;

	dest[1] = 2.0f*x*y - 2.0f*z*w;
	dest[5] = 1.0f - 2.0f*x*x - 2.0f*z*z;
	dest[9] = 2.0f*z*y + 2.0f*x*w;
	dest[13] = 0.0f;

	dest[2] = 2.0f*x*z + 2.0f*y*w;
	dest[6] = 2.0f*z*y - 2.0f*x*w;
	dest[10] = 1.0f - 2.0f*x*x - 2.0f*y*y;
	dest[14] = 0.0f;

	dest[3] = 0.f;
	dest[7] = 0.f;
	dest[11] = 0.f;
	dest[15] = 1.f;
}



// Inverts this Quaternion
inline Quaternion& Quaternion::makeInverse()
{
	x = -x; y = -y; z = -z;
	return *this;
}

// sets new Quaternion
inline Quaternion& Quaternion::set(f32 xx, f32 yy, f32 zz, f32 ww)
{
	x = xx;
	y = yy;
	z = zz;
	w = ww;
	return *this;
}


// sets new Quaternion based on euler angles
inline Quaternion& Quaternion::set(f32 xx, f32 yy, f32 zz)
{
	f64 angle;

	angle = xx * 0.5;
	const f64 sr = sin(angle);
	const f64 cr = cos(angle);

	angle = yy * 0.5;
	const f64 sp = sin(angle);
	const f64 cp = cos(angle);

	angle = zz * 0.5;
	const f64 sy = sin(angle);
	const f64 cy = cos(angle);

	const f64 cpcy = cp * cy;
	const f64 spcy = sp * cy;
	const f64 cpsy = cp * sy;
	const f64 spsy = sp * sy;

	x = (f32)(sr * cpcy - cr * spsy);
	y = (f32)(cr * spcy + sr * cpsy);
	z = (f32)(cr * cpsy - sr * spcy);
	w = (f32)(cr * cpcy + sr * spsy);

	return normalize();
}

// sets new Quaternion based on euler angles
inline Quaternion& Quaternion::set(const Vector3df& vec)
{
	return set(vec.x, vec.y, vec.z);
}

// normalizes the Quaternion
inline Quaternion& Quaternion::normalize()
{
	const f32 n = x*x + y*y + z*z + w*w;

	if (n == 1)
		return *this;

	//n = 1.0f / sqrtf(n);
	return (*this *= reciprocal_squareroot ( n ));
}


// set this Quaternion to the result of the interpolation between two quaternions
inline Quaternion& Quaternion::slerp(Quaternion q1, Quaternion q2, f32 time)
{
	f32 angle = q1.dotProduct(q2);

	if (angle < 0.0f)
	{
		q1 *= -1.0f;
		angle *= -1.0f;
	}

	f32 scale;
	f32 invscale;

	if ((angle + 1.0f) > 0.05f)
	{
		if ((1.0f - angle) >= 0.05f) // spherical interpolation
		{
			const f32 theta = acosf(angle);
			const f32 invsintheta = reciprocal(sinf(theta));
			scale = sinf(theta * (1.0f-time)) * invsintheta;
			invscale = sinf(theta * time) * invsintheta;
		}
		else // linear interploation
		{
			scale = 1.0f - time;
			invscale = time;
		}
	}
	else
	{
		q2.set(-q1.y, q1.x, -q1.w, q1.z);
		scale = sinf(PI * (0.5f - time));
		invscale = sinf(PI * time);
	}

	return (*this = (q1*scale) + (q2*invscale));
}


// calculates the dot product
inline f32 Quaternion::dotProduct(const Quaternion& q2) const
{
	return (x * q2.x) + (y * q2.y) + (z * q2.z) + (w * q2.w);
}


//! axis must be unit length
//! angle in radians
inline Quaternion& Quaternion::fromAngleAxis(f32 angle, const Vector3df& axis)
{
	const f32 fHalfAngle = 0.5f*angle;
	const f32 fSin = sinf(fHalfAngle);
	w = cosf(fHalfAngle);
	x = fSin*axis.x;
	y = fSin*axis.y;
	z = fSin*axis.z;
	return *this;
}


inline void Quaternion::toAngleAxis(f32 &angle, Vector3df &axis) const
{
	const f32 scale = sqrtf(x*x + y*y + z*z);

	if (iszero(scale) || w > 1.0f || w < -1.0f)
	{
		angle = 0.0f;
		axis.x = 0.0f;
		axis.y = 1.0f;
		axis.z = 0.0f;
	}
	else
	{
		const f32 invscale = reciprocal(scale);
		angle = 2.0f * acosf(w);
		axis.x = x * invscale;
		axis.y = y * invscale;
		axis.z = z * invscale;
	}
}

inline void Quaternion::toEuler(Vector3df& euler) const
{
	const f64 sqw = w*w;
	const f64 sqx = x*x;
	const f64 sqy = y*y;
	const f64 sqz = z*z;

	// heading = rotation about z-axis
	euler.z = (f32) (atan2(2.0 * (x*y +z*w),(sqx - sqy - sqz + sqw)));

	// bank = rotation about x-axis
	euler.x = (f32) (atan2(2.0 * (y*z +x*w),(-sqx - sqy + sqz + sqw)));

	// attitude = rotation about y-axis
	euler.y = asinf( clamp(-2.0f * (x*z - y*w), -1.0f, 1.0f) );
}


inline Vector3df Quaternion::operator* (const Vector3df& v) const
{
	// nVidia SDK implementation

	Vector3df uv, uuv;
	Vector3df qvec(x, y, z);
	uv = qvec.crossProduct(v);
	uuv = qvec.crossProduct(uv);
	uv *= (2.0f * w);
	uuv *= 2.0f;

	return v + uv + uuv;
}

// set Quaternion to identity
inline Quaternion& Quaternion::makeIdentity()
{
	w = 1.f;
	x = 0.f;
	y = 0.f;
	z = 0.f;
	return *this;
}

inline Quaternion& Quaternion::rotationFromTo(const Vector3df& from, const Vector3df& to)
{
	// Based on Stan Melax's article in Game Programming Gems
	// Copy, since cannot modify local
	Vector3df v0 = from;
	Vector3df v1 = to;
	v0.normalize();
	v1.normalize();

	const f32 d = v0.dotProduct(v1);
	if (d >= 1.0f) // If dot == 1, vectors are the same
	{
		return makeIdentity();
	}

	const f32 s = sqrtf( (1+d)*2 ); // optimize inv_sqrt
	const f32 invs = 1.f / s;
	const Vector3df c = v0.crossProduct(v1)*invs;

	x = c.x;
	y = c.y;
	z = c.z;
	w = s * 0.5f;

	return *this;
}

Q_DECLARE_METATYPE(Quaternion)

#endif

