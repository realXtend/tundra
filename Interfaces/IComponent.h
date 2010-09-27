/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IComponent.h
 *  @brief  Base class for all components. Inherit from this class when creating new components.
 */

#ifndef incl_Interfaces_IComponent_h
#define incl_Interfaces_IComponent_h

#include "ForwardDefines.h"
#include "AttributeChangeType.h"
#include "IAttribute.h"
#include "IEventData.h"
#include "CoreTypes.h"

#include <QObject>

#include <set>

class QDomDocument;
class QDomElement;

//! Base class for all components. Inherit from this class when creating new components.
/*! Use the IComponent typedef to refer to the abstract component type.
*/
class IComponent: public QObject
{
    friend class ::IAttribute;

    Q_OBJECT
    Q_PROPERTY(QString Name READ Name WRITE SetName)
    Q_PROPERTY(QString TypeName READ TypeName)

public:
    //! Constuctor.
    explicit IComponent(Foundation::Framework* framework);

    //! Copy-constructor.
    IComponent(const IComponent& rhs);

    //! Destructor.
    virtual ~IComponent();

    //! Returns type name of the component.
    virtual const QString &TypeName() const = 0;

    //! Returns name of the component.
    const QString Name() const { return name_; }

    //! Returns framework
    Foundation::Framework *GetFramework() const { return framework_; }

    //! Sets name of the component.
    void SetName(const QString& name);

    //! Sets parent entity for this component.
    void SetParentEntity(Scene::Entity* entity);

    //! Sets network sync enabled/disabled. By default on.
    void SetNetworkSyncEnabled(bool enabled);

    //! Gets whether network enabled/disabled.
    bool GetNetworkSyncEnabled() const { return network_sync_; }

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
    IAttribute* GetAttribute(const std::string &name) const;

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

    //! Attribute has changed. Send component & scenemanager change notifications
    /*! Called by IAttribute::Changed.
     */
    void AttributeChanged(IAttribute* attribute, AttributeChange::Type change);

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

    /** Handles an event. Override in your own module if you want to receive events. Do not call.
        @param category_id Category id of the event
        @param event_id Id of the event
        @param data Event data, or 0 if no data passed.
        @return True if the event was handled and is not to be propagated further.
        For more information, see @ref EventSystem.
    */
    virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data) { return false; }

public slots:
    //! Component has changed. Send notification & queue network replication as necessary
    /*! Note: call this when you're satisfied & done with your current modifications
     */
    void ComponentChanged(AttributeChange::Type change);

    //! Returns parent entity of this component.
    //! \note Returns null if called in the component's constuctor because the parent entity is not yet set there.
    Scene::Entity* GetParentEntity() const;

signals:
    //! Signal when component data has changed. Often used internally to sync eg. renderer state with EC
    void OnChanged();

    //! Signal when a single attribute has changed.
    /*! Note: also a scenemanager change signal will be sent, but this is a way to hook to a specific component
     */
    void OnAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    //! Signal when component name has been changed.
    /*  /param new_name new component name.
     *  /param old_name old component name.
     */
    void OnComponentNameChanged(const QString &new_name, const QString &old_name);

    //! Emitted when the parent entity is set.
    void ParentEntitySet();

    //! Emitted when the parent entity detached from this component, i.e. set to null.
    void ParentEntityDetached();

protected:
    //! Helper function for starting component serialization.
    /*! Creates a component element with name, adds it to the document, and returns it
     */
    QDomElement BeginSerialization(QDomDocument& doc, QDomElement& base_element) const;

    //! Helper function for adding an attribute to the component xml serialization
    void WriteAttribute(QDomDocument& doc, QDomElement& comp_element, const QString& name, const QString& value) const;

    //! Helper function for adding an attribute and it's type to the component xml serialization.
    void WriteAttribute(QDomDocument& doc, QDomElement& comp_element, const QString& name, const QString& value, const QString &type) const;

    //! Helper function for starting deserialization. 
    /*! Checks that xml element contains the right kind of EC, and if it is right, sets the component name.
        Otherwise returns false and does nothing.
     */
    bool BeginDeserialization(QDomElement& comp_element);

    //! Helper function for getting an attribute from serialized component
    QString ReadAttribute(QDomElement& comp_element, const QString &name) const;

    //! Helper function for getting a attribute type from serialized component.
    QString ReadAttributeType(QDomElement& comp_element, const QString &name) const;

    //! Pointer to parent entity (null if not attached to any entity)
    Scene::Entity* parent_entity_;

    //! Name for further identification of EC. By default empty
    QString name_;

    //! Attribute list for introspection/reflection
    AttributeVector attributes_;

    //! Change status for the component itself
    AttributeChange::Type change_;

    //! Network sync enable flag
    bool network_sync_;

    //! Framework pointer. Needed so that component is able to perform important uninitialization etc. even when not in an entity
    Foundation::Framework* framework_;

private:
    //! Called by IAttribute on initialization of each attribute
    void AddAttribute(IAttribute* attr) { attributes_.push_back(attr); }
};

#endif
