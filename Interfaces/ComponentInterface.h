// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentInterface_h
#define incl_Interfaces_ComponentInterface_h

#include "CoreDefines.h"
#include "ComponentFactoryInterface.h"
#include "ComponentRegistrarInterface.h"
#include "CoreModuleApi.h"

#include <QObject>
#include <QDomDocument>

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
        
        virtual void SerializeTo(QDomDocument& doc, QDomElement& base_element) const {};
        virtual void DeserializeFrom(QDomElement& element) {};
        
    private:
        ComponentInterface();
        
    protected:
        //! Helper function for starting component serialization. Creates a component element with name, adds it to the document, and returns it
        QDomElement BeginSerialization(QDomDocument& doc, QDomElement& base_element) const;
        //! Helper function for adding an attribute to the component xml serialization
        void WriteAttribute(QDomDocument& doc, QDomElement& comp_element, const std::string& name, const std::string& value) const;
        //! Helper function for verifying that an xml element contains the right kind of EC, before starting to deserialize
        bool VerifyComponentType(QDomElement& comp_element) const;
        //! Helper function for getting an attribute from serialized component
        std::string ReadAttribute(QDomElement& comp_element, const std::string& name) const;
        
        //! Pointer to framework
        Foundation::Framework* framework_;
        //! Pointer to parent entity (null if not attached to any entity)
        Scene::Entity* parent_entity_;
        
        //! Signal when component data has changed, probably as result of deserialization
    signals:
        void OnChanged();
        
    };
}

#endif
