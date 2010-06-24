/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ComponentInterface.h
 *  @brief  Base class for all components. Inherit from this class when creating new components.
 */

#ifndef incl_Interfaces_ComponentInterface_h
#define incl_Interfaces_ComponentInterface_h

//#include "CoreDefines.h"
//#include "ComponentFactoryInterface.h"
//#include "ComponentRegistrarInterface.h"
//#include "CoreModuleApi.h"

#include "AttributeChangeType.h"
#include "AttributeInterface.h"

#include <QObject>

#include <vector>
#include <set>

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

    //! Base class for all components. Inherit from this class when creating new components.
    /*! Use the ComponentInterface typedef to refer to the abstract component type.
    */
    class ComponentInterface : public QObject
    {
        friend class AttributeInterface;

        Q_OBJECT

    public:
        //! Default constuctor.
        ComponentInterface();

        //! Copy-constructor.
        ComponentInterface(const ComponentInterface& rhs);

        //! Destructor.
        virtual ~ComponentInterface();

        //! Returns type name of the component.
        virtual const std::string &TypeName() const = 0;

        //! Returns the framework pointer.
        Framework* GetFramework() const;

        //! Returns name of the component.
        const std::string& Name() const { return name_; }

        //! Sets name of the component.
        void SetName(const std::string& name) { name_ = name; }

        //! Sets parent entity for this component.
        void SetParentEntity(Scene::Entity* entity);

        //! Returns parent entity of this component.
        //! \note Returns null if called in the component's constuctor because the parent entity is not yet set there.
        Scene::Entity* GetParentEntity() const;

        //! Return true for components that support XML serialization
        virtual bool IsSerializable() const { return false; }
        //! Get number of attributes in this component.
        int GetNumberOfAttributes() const { return attributes_.size(); }
        //! Return attributes of this component for reflection
        const AttributeVector& GetAttributes() const { return attributes_; }

        //! Iterate throught the attribute vector and try to find the match for name string.
        /*! If attribute with same name is found return attribute interface pointer, else return null.
         *  @param name Attribute name.
         */
        AttributeInterface* GetAttribute(const std::string &name) const;

        /*! Return pointer to atribute with spesific name and typename/class or null if the attribute not found.
            \param T Typename/class of the attribute.
            \param name Attribute name.
            \note Always remember to check for null pointer after retrieving the attribute.
         */
        template<typename T> Attribute<T> *GetAttribute(const std::string &name) const
        {
            for(size_t i = 0; i < attributes_.size(); ++i)
                if (attributes_[i]->GetNameString() == name)
                    return dynamic_cast<Attribute<T> *>(&attributes_[i]);
            return 0;
        }

        //! Component has changed. Send notification & queue network replication as necessary
        /*! Note: call this when you're satisfied & done with your current modifications
         */
        void ComponentChanged(AttributeChange::Type change);

        //! Attribute has changed. Send component & scenemanager change notifications
        /*! Called by AttributeInterface::Changed.
         */
        void AttributeChanged(AttributeInterface* attribute, AttributeChange::Type change);

        //! Read change status of the component
        AttributeChange::Type GetChange() const { return change_; }

        //! Reset change status of component and all attributes
        /*! Called by serialization managers when they have managed syncing the component
         */
        void ResetChange();

        //! Serialize to XML
        virtual void SerializeTo(QDomDocument& doc, QDomElement& base_element) const;

        //! Deserialize from XML
        virtual void DeserializeFrom(QDomElement& element, AttributeChange::Type change);

    signals:
        //! Signal when component data has changed. Often used internally to sync eg. renderer state with EC
        void OnChanged();

        //! Signal when a single attribute has changed.
        /*! Note: also a scenemanager change signal will be sent, but this is a way to hook to a specific component
         */
        void OnAttributeChanged(AttributeInterface* attribute, AttributeChange::Type change);

        //! Emitted when the parent entity is set.
        void ParentEntitySet();

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

        //! Pointer to parent entity (null if not attached to any entity)
        Scene::Entity* parent_entity_;

        //! Name for further identification of EC. By default empty
        std::string name_;

        //! Attribute list for introspection/reflection
        AttributeVector attributes_;

        //! Change status for the component itself
        AttributeChange::Type change_;

    private:
        //! Called by AttributeInterface on initialization of each attribute
        void AddAttribute(AttributeInterface* attr) { attributes_.push_back(attr); }
    };
}

#endif
