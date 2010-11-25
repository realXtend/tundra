// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QuaternionDecorator.h"

namespace PythonScript {

    Quaternion* QuaternionDecorator::new_Quaternion(const float x, const float y, const float z, const float w) { return new Quaternion(x, y, z, w); }
    Quaternion* QuaternionDecorator::new_Quaternion(const float x, const float y, const float z) { return new Quaternion(x, y, z); }
    Quaternion* QuaternionDecorator::new_Quaternion(const Vector3df& vec) { return new Quaternion(vec); }
    Quaternion* QuaternionDecorator::new_Quaternion(const float angle, const Vector3df& vec) { return new Quaternion(angle, vec); }
    void QuaternionDecorator::delete_Quaternion(Quaternion* self) { delete self; }

    float QuaternionDecorator::x(Quaternion* self) { return self->x; }
    float QuaternionDecorator::y(Quaternion* self) { return self->y; }
    float QuaternionDecorator::z(Quaternion* self) { return self->z; } 
    float QuaternionDecorator::w(Quaternion* self) { return self->w; } 

    void QuaternionDecorator::setx(Quaternion* self, float value) { self->x = value; }
    void QuaternionDecorator::sety(Quaternion* self, float value) { self->y = value; }
    void QuaternionDecorator::setz(Quaternion* self, float value) { self->z = value; }
    void QuaternionDecorator::setw(Quaternion* self, float value) { self->w = value; }

    void QuaternionDecorator::toEuler(Quaternion* self, Vector3df& euler) { 
        self->toEuler(euler);
    }
    
    void QuaternionDecorator::set(Quaternion* self, const Vector3df& euler) {
        self->set(euler);
    }
}

