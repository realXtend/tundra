// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptModule_Vector3dfDecorator_h
#define incl_PythonScriptModule_Vector3dfDecorator_h

#include <QObject>
#include "Core.h"

namespace PythonScript
{
    class Vector3dfDecorator : public QObject
    {
        Q_OBJECT

    public slots:
        Vector3df* new_Vector3df(const float x, const float y, const float z);
        Vector3df* new_Vector3df();
        void delete_Vector3df(Vector3df *self);

        float x(Vector3df* self);
        float y(Vector3df* self);
        float z(Vector3df* self);
        
        void setx(Vector3df* self, float value);
        void sety(Vector3df* self, float value);
        void setz(Vector3df* self, float value);
        
        float getLength(const Vector3df* self);
        float getLengthSQ(const Vector3df* self);
        
        void setLength(Vector3df* self, float length);
        
        float dotProduct(const Vector3df* self, const Vector3df& other);
        
        float getDistanceFrom(const Vector3df* self, const Vector3df& second);
        float getDistanceFromSQ(const Vector3df* self, const Vector3df& second);
        
        Vector3df crossProduct(const Vector3df* self, const Vector3df& other);
        
        bool isBetweenPoints(const Vector3df* self, const Vector3df& begin, const Vector3df& end);
        
        void normalize(Vector3df* self);
        
        void invert(Vector3df* self);
        
        void rotateXZBy(Vector3df* self, float degrees, const Vector3df& center);
        void rotateXYBy(Vector3df* self, float degrees, const Vector3df& center);
        void rotateYZBy(Vector3df* self, float degrees, const Vector3df& center);
        
        Vector3df getInterpolated(Vector3df* self, const Vector3df& vec, float factor);
        Vector3df getInterpolated_quadratic(Vector3df* self, const Vector3df& vec2, const Vector3df& vec3, float factor);
        void interpolate(Vector3df* self, const Vector3df& vec1, const Vector3df& vec2, float factor);
        
        Vector3df getHorizontalAngle(Vector3df* self);
        Vector3df rotationToDirection(Vector3df* self, const Vector3df& vec = Vector3df(0,0,1));
    };
}

#endif

