// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptModule_TransformDecorator_h
#define incl_PythonScriptModule_TransformDecorator_h

#include <QObject>
#include "Core.h"
#include "Transform.h"

namespace PythonScript
{
    class TransformDecorator : public QObject
    {
    Q_OBJECT

    public slots:
        Transform* new_Transform(const Vector3df &pos, const Vector3df &rot, const Vector3df &scale);
        Transform* new_Transform();
        void delete_Transform(Transform* self);

        Vector3df position(const Transform* self);
        Vector3df rotation(const Transform* self);
        Vector3df scale(const Transform* self);

        void SetPos(Transform* self, float x, float y, float z);		
        void SetPos(Transform* self, Vector3df pos);

        void SetRot(Transform* self, float x, float y, float z);
        void SetRot(Transform* self, Vector3df rot);

        void SetScale(Transform* self, float x, float y, float z);
        void SetScale(Transform* self, Vector3df scale);
	};
}

#endif
