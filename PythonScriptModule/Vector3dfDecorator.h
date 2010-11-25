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
        float getDistanceFrom(const Vector3df* first, const Vector3df& second);
    };
}

#endif

