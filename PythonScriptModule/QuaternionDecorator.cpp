// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "QuaternionDecorator.h"

namespace PythonScript {

    Quaternion* QuaternionDecorator::new_Quaternion(const float x, const float y, const float z, const float w)
    {
        return new Quaternion(x, y, z, w);
    }
    
    Quaternion* QuaternionDecorator::new_Quaternion(const float x, const float y, const float z)
    {
        return new Quaternion(x, y, z);
    }
    
    Quaternion* QuaternionDecorator::new_Quaternion(const Vector3df& vec)
    {
        return new Quaternion(vec);
    }
    
    Quaternion* QuaternionDecorator::new_Quaternion(const float angle, const Vector3df& vec)
    {
        return new Quaternion(angle, vec);
    }
    
    void QuaternionDecorator::delete_Quaternion(Quaternion* self)
    {
        delete self;
    }

    float QuaternionDecorator::x(Quaternion* self)
    {
        return self->x;
    }
    
    float QuaternionDecorator::y(Quaternion* self)
    {
        return self->y;
    }
    
    float QuaternionDecorator::z(Quaternion* self)
    {
        return self->z;
    }
    
    float QuaternionDecorator::w(Quaternion* self)
    {
        return self->w;
    }
    
    bool QuaternionDecorator::equals(Quaternion* self, const Quaternion& other, const float tolerance)
    {
        return self->equals(other, tolerance);
    }

    void QuaternionDecorator::setx(Quaternion* self, float value)
    {
        self->x = value;
    }
    
    void QuaternionDecorator::sety(Quaternion* self, float value)
    {
        self->y = value;
    }
    
    void QuaternionDecorator::setz(Quaternion* self, float value)
    {
        self->z = value;
    }
    
    void QuaternionDecorator::setw(Quaternion* self, float value)
    {
        self->w = value;
    }
    
    float QuaternionDecorator::dotProduct(Quaternion* self, const Quaternion& other)
    {
        return self->dotProduct(other);
    }

	Quaternion* QuaternionDecorator::product(Quaternion* self, const Quaternion& other)
	{
		Quaternion tmp = *self * other;
		return new Quaternion(tmp.x, tmp.y, tmp.z, tmp.w);
	}
	
	Vector3df* QuaternionDecorator::product(Quaternion* self, const Vector3df& vec)
	{
		Vector3df tmp = *self * vec;
		return new Vector3df(tmp);
	}

    void QuaternionDecorator::toEuler(Quaternion* self, Vector3df& euler)
    {
        self->toEuler(euler);
    }
    
    void QuaternionDecorator::set(Quaternion* self, float x, float y, float z, float w)
    {
        self->set(x, y, z, w);
    }
    
    void QuaternionDecorator::set(Quaternion* self, float x, float y, float z)
    {
        self->set(x, y, z);
    }
    
    void QuaternionDecorator::set(Quaternion* self, const Vector3df& euler)
    {
        self->set(euler);
    }
    
    void QuaternionDecorator::normalize(Quaternion* self)
    {
        self->normalize();
    }
    
    void QuaternionDecorator::makeInverse(Quaternion* self)
    {
        self->makeInverse();
    }
    
    void QuaternionDecorator::slerp(Quaternion* self, Quaternion q1, Quaternion q2, float interpolate)
    {
        self->slerp(q1, q2, interpolate);
    }
    
    void QuaternionDecorator::fromAngleAxis(Quaternion* self, float angle, const Vector3df& axis)
    {
        self->fromAngleAxis(angle, axis);
    }
    
    void QuaternionDecorator::toAngleAxis(Quaternion* self, float& angle, Vector3df& axis)
    {
        self->toAngleAxis(angle, axis);
    }
    
    void QuaternionDecorator::makeIdentity(Quaternion* self)
    {
        self->makeIdentity();
    }
    
    void QuaternionDecorator::rotationFromTo(Quaternion* self, const Vector3df& from, const Vector3df& to)
    {
        self->rotationFromTo(from, to);
    }
}

