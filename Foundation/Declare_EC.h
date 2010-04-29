// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_EC_Define_h
#define incl_Foundation_EC_Define_h

//#include "ComponentInterface.h"
#include "ComponentManager.h"
//#include "ForwardDefines.h"
#include "ComponentRegistrarInterface.h"
#include "ComponentFactoryInterface.h"
#include "Framework.h"

namespace Foundation
{
    class ModuleInterface;
}

//! Helper macro for creating new entity components
/*!
    \ingroup Scene_group
*/
#define DECLARE_EC(component)                                                               \
  public:                                                                                   \
    class component##Registrar : public Foundation::ComponentRegistrarInterface             \
    {                                                                                       \
    public:                                                                                 \
        component##Registrar() {}                                                           \
        virtual ~component##Registrar() {}                                                  \
                                                                                            \
        virtual void Register(Foundation::Framework *framework,                             \
            Foundation::ModuleInterface* module)                                            \
        {                                                                                   \
            component::RegisterComponent(framework, module);                                \
        }                                                                                   \
        virtual void Unregister(Foundation::Framework *framework)                           \
        {                                                                                   \
            component::UnregisterComponent(framework);                                      \
        }                                                                                   \
    };                                                                                      \
                                                                                            \
  private:                                                                                  \
    class component##Factory : public Foundation::ComponentFactoryInterface                 \
    {                                                                                       \
    public:                                                                                 \
        component##Factory(Foundation::ModuleInterface* module) : module_(module) {}        \
        virtual ~component##Factory() {}                                                    \
                                                                                            \
        virtual Foundation::ComponentInterfacePtr  operator()()                             \
        {                                                                                   \
            return Foundation::ComponentInterfacePtr(new component(module_));               \
        }                                                                                   \
                                                                                            \
        virtual Foundation::ComponentInterfacePtr  operator()(                              \
                    const Foundation::ComponentInterfacePtr &other)                         \
        {                                                                                   \
           return Foundation::ComponentInterfacePtr(                                        \
                    new component(*dynamic_cast<component*>(other.get())));                 \
        }                                                                                   \
    private:                                                                                \
        Foundation::ModuleInterface* module_;                                               \
    };                                                                                      \
                                                                                            \
    friend class component##Factory;                                                        \
                                                                                            \
  public:                                                                                   \
    static void RegisterComponent(const Foundation::Framework *framework,                   \
        Foundation::ModuleInterface* module)                                                \
    {                                                                                       \
        Foundation::ComponentFactoryInterfacePtr factory =                                  \
            Foundation::ComponentFactoryInterfacePtr(new component##Factory(module));       \
        framework->GetComponentManager()->RegisterFactory(TypeNameStatic(), factory);       \
    }                                                                                       \
                                                                                            \
    static void UnregisterComponent(const Foundation::Framework *framework)                 \
    {                                                                                       \
        framework->GetComponentManager()->UnregisterFactory(TypeNameStatic());              \
    }                                                                                       \
                                                                                            \
    static const std::string &TypeNameStatic()                                              \
    {                                                                                       \
        static const std::string name(#component);                                          \
        return name;                                                                        \
    }                                                                                       \
                                                                                            \
    virtual const std::string &TypeName() const                                             \
    {                                                                                       \
        return component::TypeNameStatic();                                                 \
    }                                                                                       \
  private:                                                                                  \

#endif
