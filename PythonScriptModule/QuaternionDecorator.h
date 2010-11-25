// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptModule_QuaternionDecorator_h
#define incl_PythonScriptModule_QuaternionDecorator_h

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

            void setx(Quaternion* self, float value);
            void sety(Quaternion* self, float value);
            void setz(Quaternion* self, float value);
            void setw(Quaternion* self, float value);

            void toEuler(Quaternion* self, Vector3df& euler);
            void set(Quaternion* self, const Vector3df& euler);
    };
}

#endif

