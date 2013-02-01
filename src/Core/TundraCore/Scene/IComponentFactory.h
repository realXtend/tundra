// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "IComponent.h"

#include <QString>

/// A common interface for factories which instantiate components of different types.
class TUNDRACORE_API IComponentFactory
{
public:
    IComponentFactory() {}
    virtual ~IComponentFactory() {}

    virtual const QString &TypeName() const = 0;
    virtual u32 TypeId() const = 0;
    virtual ComponentPtr Create(Scene* scene, const QString &newComponentName) const = 0;
//    virtual ComponentPtr Clone(IComponent *existingComponent, const QString &newComponentName) const = 0;
};

/// A factory for instantiating components of a templated type T.
template<typename T>
class GenericComponentFactory : public IComponentFactory
{
public:
    const QString &TypeName() const { return T::TypeNameStatic(); }
    u32 TypeId() const { return T::TypeIdStatic(); }

    ComponentPtr Create(Scene* scene, const QString &newComponentName) const
    {
        ComponentPtr component = MAKE_SHARED(T, scene);
        component->SetName(newComponentName);
        return component;
    }
/*     ///\todo Implement this.

    ComponentPtr Clone(IComponent *existingComponent, const QString &newComponentName) const
    {
        if (!existingComponent)
        {
            LogError("Cannot clone component from a null pointer!");
            return ComponentPtr();
        }

        T *existing = dynamic_cast<T*>(existingComponent);
        if (!existing)
        {
            LogError("Cannot clone component of type \"" + TypeName() + " from a component of type \"" + existingComponent->TypeName() + "\"!");
            return ComponentPtr();
        }
//        ComponentPtr component = MAKE_SHARED(T, *existingComponent);
        ComponentPtr component = shared_ptr<T>(new T, *existingComponent);
        component->SetName(newComponentName);
        return component;
    }
    */
};
