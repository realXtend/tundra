// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Vector3dfDecorator.h"

namespace PythonScript {
    Vector3df* Vector3dfDecorator::new_Vector3df(const float x, const float y, const float z) { return new Vector3df(x, y, z); }
    Vector3df* Vector3dfDecorator::new_Vector3df() { return new Vector3df(); }
    void Vector3dfDecorator::delete_Vector3df(Vector3df *self) { delete self; }

    float Vector3dfDecorator::x(Vector3df* self) { return self->x; }
    float Vector3dfDecorator::y(Vector3df* self) { return self->y; }
    float Vector3dfDecorator::z(Vector3df* self) { return self->z; } 

    void Vector3dfDecorator::setx(Vector3df* self, float value) { self->x = value; }
    void Vector3dfDecorator::sety(Vector3df* self, float value) { self->y = value; }
    void Vector3dfDecorator::setz(Vector3df* self, float value) { self->z = value; }
    float Vector3dfDecorator::getDistanceFrom(const Vector3df* first, const Vector3df& second) { return first->getDistanceFrom(second); }
}
