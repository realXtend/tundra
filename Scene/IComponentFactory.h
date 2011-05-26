// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Scene_IComponentFactory_h
#define incl_Scene_IComponentFactory_h

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <QString>
#include "CoreTypes.h"
#include "LoggingFunctions.h"

class IComponent;

class IComponentFactory
{
public:
    IComponentFactory() {}
    virtual ~IComponentFactory() {}

    virtual QString TypeName() = 0;
    virtual u32 TypeId() = 0;
    virtual boost::shared_ptr<IComponent> Create(Scene* scene, const QString &newComponentName) = 0;
//    virtual boost::shared_ptr<IComponent> Clone(IComponent *existingComponent, const QString &newComponentName) = 0;
};

template<typename T>
class GenericComponentFactory : public IComponentFactory
{
public:
    QString TypeName() { return T::TypeNameStatic(); }
    u32 TypeId() { return T::TypeIdStatic(); }

    boost::shared_ptr<IComponent> Create(Scene* scene, const QString &newComponentName)
    {
        boost::shared_ptr<IComponent> component = boost::make_shared<T>(scene);
        component->SetName(newComponentName);
        return component;
    }
/*     ///\todo Implement this.

    boost::shared_ptr<IComponent> Clone(IComponent *existingComponent, const QString &newComponentName)
    {
        if (!existingComponent)
        {
            LogError("Cannot clone component from a null pointer!");
            return boost::shared_ptr<IComponent>();
        }

        T *existing = dynamic_cast<T*>(existingComponent);
        if (!existing)
        {
            LogError("Cannot clone component of type \"" + TypeName() + " from a component of type \"" + existingComponent->TypeName() + "\"!");
            return boost::shared_ptr<IComponent>();
        }
//        boost::shared_ptr<T> component = boost::make_shared<T>(*existingComponent);
        boost::shared_ptr<T> component = boost::shared_ptr<T>(new T(*existingComponent));
        component->SetName(newComponentName);
        return component;
    }
    */
};

#endif
