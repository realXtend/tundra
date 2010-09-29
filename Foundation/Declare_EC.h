// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_EC_Define_h
#define incl_Foundation_EC_Define_h

#include "ComponentManager.h"
#include "IComponentRegistrar.h"
#include "IComponentFactory.h"
#include "Framework.h"

class IModule;

//! Helper macro for creating new entity components
/*!
    \ingroup Scene_group
*/
#define DECLARE_EC(component)                                                           \
  public:                                                                               \
    class component##Registrar : public IComponentRegistrar                             \
    {                                                                                   \
    public:                                                                             \
        component##Registrar() {}                                                       \
        virtual ~component##Registrar() {}                                              \
                                                                                        \
        virtual void Register(Foundation::Framework *framework,                         \
            IModule* module)                                                            \
        {                                                                               \
            component::RegisterComponent(framework, module);                            \
        }                                                                               \
        virtual void Unregister(Foundation::Framework *framework)                       \
        {                                                                               \
            component::UnregisterComponent(framework);                                  \
        }                                                                               \
    };                                                                                  \
                                                                                        \
  private:                                                                              \
    class component##Factory : public IComponentFactory                                 \
    {                                                                                   \
    public:                                                                             \
        component##Factory(IModule* module) : module_(module) {}                        \
        virtual ~component##Factory() {}                                                \
                                                                                        \
        virtual ComponentPtr  operator()()                                     \
        {                                                                               \
            return ComponentPtr(new component(module_));                       \
        }                                                                               \
                                                                                        \
        virtual ComponentPtr  operator()(                                      \
                    const ComponentPtr &other)                                 \
        {                                                                               \
           return ComponentPtr(                                                \
                    new component(*dynamic_cast<component*>(other.get())));             \
        }                                                                               \
    private:                                                                            \
        IModule* module_;                                                               \
    };                                                                                  \
                                                                                        \
    friend class component##Factory;                                                    \
                                                                                        \
  public:                                                                               \
    static void RegisterComponent(const Foundation::Framework *framework,               \
        IModule* module)                                                                \
    {                                                                                   \
        ComponentFactoryPtr factory =                                                   \
            ComponentFactoryPtr(new component##Factory(module));                        \
        framework->GetComponentManager()->RegisterFactory(TypeNameStatic(), factory);   \
    }                                                                                   \
                                                                                        \
    static void UnregisterComponent(const Foundation::Framework *framework)             \
    {                                                                                   \
        framework->GetComponentManager()->UnregisterFactory(TypeNameStatic());          \
    }                                                                                   \
                                                                                        \
    static const QString &TypeNameStatic()                                              \
    {                                                                                   \
        static const QString name(#component);                                          \
        return name;                                                                    \
    }                                                                                   \
                                                                                        \
    virtual const QString &TypeName() const                                             \
    {                                                                                   \
        return component::TypeNameStatic();                                             \
    }                                                                                   \
  private:                                                                              \

//Q_PROPERTY(type attribute READ get##attribute WRITE set##attribute)
/// Exposes an existing 'Attribute<type> attribute' member as an automatically generated QProperty of name 'attribute'.
#define EXPOSE_ATTRIBUTE_AS_QPROPERTY(type, attribute) \
    type get##attribute() const { return (type)attribute.Get(); } \
    void set##atribute(type value) { attribute.Set((type)value, AttributeChange::Local); }

//Q_PROPERTY(type attribute READ get##attribute WRITE set##attribute)
/// Defines a new 'Attribute<type> attribute' member as an automatically generated QProperty of name 'attribute'.
#define DEFINE_QPROPERTY_ATTRIBUTE(type, attribute) \
    Attribute<type > attribute; \
    type get##attribute() const { return (type)attribute.Get(); } \
    void set##attribute(type value) { attribute.Set((type)value, AttributeChange::Local); }


#endif
