// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <PythonQt.h>
#include <QObject>
#include "Core.h"

namespace PythonScript
{
    class QuaternionDecorator : public QObject
    {    
        Q_OBJECT
        
        public slots:
            Quaternion* new_Quaternion(const float x, const float y, const float z, const float w);
            Quaternion* new_Quaternion(const float x, const float y, const float z);
            Quaternion* new_Quaternion(const Vector3df& vec);
            Quaternion* new_Quaternion(const float angle, const Vector3df& vec);
            void delete_Quaternion(Quaternion* self);

            float x(Quaternion* self);
            float y(Quaternion* self);
            float z(Quaternion* self);
            float w(Quaternion* self);
            
            bool equals(Quaternion* self, const Quaternion& other, const float tolerance = ROUNDING_ERROR_32);

            void setx(Quaternion* self, float value);
            void sety(Quaternion* self, float value);
            void setz(Quaternion* self, float value);
            void setw(Quaternion* self, float value);
            
            float dotProduct(Quaternion* self, const Quaternion& other);

            void set(Quaternion* self, float x, float y, float z, float w);
            void set(Quaternion* self, float x, float y, float z);
            void set(Quaternion* self, const Vector3df& euler);
            
            void normalize(Quaternion* self);
            void makeInverse(Quaternion* self);
            void slerp(Quaternion* self, Quaternion q1, Quaternion q2, float interpolate);
            
            void fromAngleAxis(Quaternion* self, float angle, const Vector3df& axis);
            
            void toEuler(Quaternion* self, Vector3df& euler);
            void toAngleAxis(Quaternion* self, float& angle, Vector3df& axis);
            
            void makeIdentity(Quaternion* self);
            
            void rotationFromTo(Quaternion* self, const Vector3df& from, const Vector3df& to);
    };
}


