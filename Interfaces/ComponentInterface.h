// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentInterface_h
#define incl_Interfaces_ComponentInterface_h

#include "CoreDefines.h"
#include "ComponentFactoryInterface.h"
#include "ComponentRegistrarInterface.h"
#include "CoreModuleApi.h"

#include <QObject>

namespace Scene
{
    class Entity;
}

namespace Foundation
{
    class Framework;

    //! Base class for all components. Inherit from this class when creating new components.
    /*! Use the ComponentInterface typedef to refer to the abstract component type.
    */
    class /*MODULE_API*/ ComponentInterface : public QObject
    {
        Q_OBJECT
        
    public:
        explicit ComponentInterface(Foundation::Framework *framework);
        ComponentInterface(const ComponentInterface &rhs);
        virtual ~ComponentInterface();
        virtual const std::string &Name() const = 0;
        Foundation::Framework* GetFramework() const { return framework_; }
        
        void SetParentEntity(Scene::Entity* entity);
        Scene::Entity* GetParentEntity() const;
        
    private:
        ComponentInterface();
        
    protected:
        //! Pointer to framework
        Foundation::Framework* framework_;
        //! Pointer to parent entity (null if not attached to any entity)
        Scene::Entity* parent_entity_;
    };
}

#endif
