// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentInterface_h
#define incl_Interfaces_ComponentInterface_h

#include "CoreDefines.h"
#include "ComponentFactoryInterface.h"
#include "ComponentRegistrarInterface.h"
#include "CoreModuleApi.h"

#include <QObject>

#include <vector>

class QDomDocument;
class QDomElement;

namespace Scene
{
    class Entity;
}

namespace Foundation
{
    class Framework;
    
    class AttributeInterface;
    typedef std::vector<AttributeInterface*> AttributeVector;
    
    //! Enumeration of attribute/component change types for replication
    enum ChangeType
    {
        //! No change: attribute/component is up to date
        None = 0,
        //! Local change that should be replicated to server
        Local,
        //! Local change that should not be replicated to server
        LocalOnly,
        //! Change that came from network
        Network
    };
    
    //! Base class for all components. Inherit from this class when creating new components.
    /*! Use the ComponentInterface typedef to refer to the abstract component type.
    */
    class /*MODULE_API*/ ComponentInterface : public QObject
    {
        friend class AttributeInterface;
        
        Q_OBJECT
        
    public:
        explicit ComponentInterface(Foundation::Framework *framework);
        ComponentInterface(const ComponentInterface& rhs);
        virtual ~ComponentInterface();
        virtual const std::string &TypeName() const = 0;
        Foundation::Framework* GetFramework() const { return framework_; }
        
        const std::string& Name() { return name_; }
        void SetName(const std::string& name) { name_ = name; }
        
        void SetParentEntity(Scene::Entity* entity);
        Scene::Entity* GetParentEntity() const;
        
        //! Return true for components that support XML serialization
        virtual bool IsSerializable() const { return false; }
        //! Return attributes of this component for reflection
        const AttributeVector& GetAttributes() const { return attributes_; }
        //! Component has changed. Send notification & queue network replication as necessary
        /*! Note: call this when you're satisfied & done with your current modifications
         */
        void ComponentChanged(ChangeType change);
        
        //! Serialize to XML
        virtual void SerializeTo(QDomDocument& doc, QDomElement& base_element) const;
        //! Deserialize from XML
        virtual void DeserializeFrom(QDomElement& element, ChangeType change);
        
    protected:
        //! Helper function for starting component serialization. Creates a component element with name, adds it to the document, and returns it
        QDomElement BeginSerialization(QDomDocument& doc, QDomElement& base_element) const;
        //! Helper function for adding an attribute to the component xml serialization
        void WriteAttribute(QDomDocument& doc, QDomElement& comp_element, const std::string& name, const std::string& value) const;
        //! Helper function for starting deserialization. 
        /*! Checks that xml element contains the right kind of EC, and if it is right, sets the component name.
            Otherwise returns false and does nothing.
         */
        bool BeginDeserialization(QDomElement& comp_element);
        //! Helper function for getting an attribute from serialized component
        std::string ReadAttribute(QDomElement& comp_element, const std::string& name) const;
        
        //! Pointer to framework
        Foundation::Framework* framework_;
        //! Pointer to parent entity (null if not attached to any entity)
        Scene::Entity* parent_entity_;
        //! Name for further identification of EC. By default empty
        std::string name_;
        //! Attribute list for introspection/reflection
        AttributeVector attributes_;
        
    signals:
        //! Signal when component data has changed. Often used internally to sync eg. renderer state with EC
        void OnChanged();
        
    private:
        ComponentInterface();
        
        //! Called by AttributeInterface on initialization of each attribute
        void AddAttribute(AttributeInterface* attr) { attributes_.push_back(attr); }
    };
}

#endif
