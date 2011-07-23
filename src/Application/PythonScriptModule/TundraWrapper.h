// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "PythonFwd.h"
#include "AttributeChangeType.h"

namespace PythonScript
{
    /** Multipurpose instance decorator class for PythonQt to add functionality to known QObject classes.
        The first parameter to all functions is the type that gets the method. 
        Example: C++    : int DoSomething(Entity *entity, int i)
                 Python : returnInt = entity.DoSomething(12)
        Registered with : PythonQt::self()->addInstanceDecorators()
     */
    class TundraInstanceDecorator : public QObject
    {

    Q_OBJECT

    public slots:
        // Scene: Member functions, this is a knows QObject for PythonQt, we are adding functionality to the existing object.
        QObject *CreateEntityLocalRaw(Scene *scene, const QStringList &components = QStringList(), AttributeChange::Type change = AttributeChange::LocalOnly, bool defaultNetworkSync = false);

        // Entity: Member functions, this is a knows QObject for PythonQt, we are adding functionality to the existing object.
        QObject *GetComponentRaw(Entity *entity, const QString &componentType);
        QObject *GetOrCreateComponentRaw(Entity *entity, const QString &componentType);
    };

    /** Multipurpose decorator class for PythonQt to resolve unknown non-QObject C++ classes.
        Example: C++    : MyTundraClass* new_MyTundraClass(const QString &msg)
                 Python : tundraClass = MyTundraClass("hey")
        Registered with : PythonQt::self()->addDecorators()
                        : PythonQt::self()->registerCPPClass("MyTundraClass");
     */
    class TundraDecorator : public QObject
    {

    Q_OBJECT

    public slots:
        // AssetReference: Construct and destruct
        AssetReference *new_AssetReference();
        AssetReference *new_AssetReference(const QString &reference);
        void delete_AssetReference(AssetReference *obj);

        // AssetReference: Functions
        QString ref(const AssetReference *obj);
        void setRef(AssetReference *obj, const QString &assetRef);
        QString type(const AssetReference *obj);
        void setType(AssetReference *obj, const QString &assetType);
    };
}
