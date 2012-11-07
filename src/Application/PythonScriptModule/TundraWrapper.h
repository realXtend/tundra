// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "PythonFwd.h"
#include "Math/float3.h"
#include "Transform.h"
#include "AttributeChangeType.h"

namespace PythonScript
{
    /// Multipurpose instance decorator class for PythonQt to add functionality to known QObject classes.
    /** The first parameter to all functions is the type that gets the method. 
        Example: C++    : int DoSomething(Entity *entity, int i)
                 Python : returnInt = entity.DoSomething(12)
        Registered with : PythonQt::self()->addInstanceDecorators() 
        @cond PRIVATE */
    class TundraInstanceDecorator : public QObject
    {
        Q_OBJECT

    public slots:
        // Scene: Member functions, this is a knows QObject for PythonQt, we are adding functionality to the existing object.
        QObject *CreateEntityLocalRaw(Scene *scene, const QStringList &components = QStringList(), AttributeChange::Type change = AttributeChange::LocalOnly, bool defaultNetworkSync = false);

        // SceneAPI too..
        QObject *GetSceneRaw(SceneAPI *sceneapi, const QString &scenename);

        // Entity: Member functions, this is a knows QObject for PythonQt, we are adding functionality to the existing object.
        QObject *GetComponentRaw(Entity *entity, const QString &componentType);
        QObject *GetOrCreateComponentRaw(Entity *entity, const QString &componentType);
    };
    /** @endcond */

    /// Multipurpose decorator class for PythonQt to resolve unknown non-QObject C++ classes.
    /** Example: C++    : MyTundraClass* new_MyTundraClass(const QString &msg)
                 Python : tundraClass = MyTundraClass("hey")
        Registered with : PythonQt::self()->addDecorators()
                        : PythonQt::self()->registerCPPClass("MyTundraClass");
        @cond PRIVATE */
    class TundraDecorator : public QObject
    {
        Q_OBJECT

    public slots:
      //QObject *get(ScenePtr self); //if can't autoconvert boost pointers, perhaps can make a generic raw pointer getter?

        // AssetReference: Construct and destruct
        AssetReference *new_AssetReference();
        AssetReference *new_AssetReference(const QString &reference);
        AssetReference *new_AssetReference(const QString &assetRef, const QString &assetType);
        void delete_AssetReference(AssetReference *obj);

        // AssetReference: Functions
        QString ref(const AssetReference *obj);
        void setRef(AssetReference *obj, const QString &assetRef);
        QString type(const AssetReference *obj);
        void setType(AssetReference *obj, const QString &assetType);

        //getters for float3 x, y and z coordinates
        float x(float3* self);
        float y(float3* self);
        float z(float3* self);

        //setters for float3 x, y and z coordinates
        void setx(float3* self, float value);
        void sety(float3* self, float value);
        void setz(float3* self, float value);

        //procedure to negate the float3 object
        float3 neg(float3* self);

        //getters for Transform pos,rot and scale float3 objects
        float3 position(const Transform* self);
        float3 rotation(const Transform* self);
        float3 scale(const Transform* self);

        //setters for Transform objects
        void SetPos(Transform* self, float x, float y, float z);
        void SetRot(Transform* self, float x, float y, float z);
        void SetScale(Transform* self, float x, float y, float z);

        float3 QuatToEulerZYX(Quat* self) const;

        //a setter for float3 objects: cannot assign back to function call
        void set(float3* self, float x, float y, float z);
        //setters for Quat
        void SetFromAxisAngle(Quat* self, const float3 &rotationAxis, const float rotationAngleRadians);
        Quat FromEulerZYX(Quat* self, float z, float y, float x) const;
     };
    /** @endcond */
}
