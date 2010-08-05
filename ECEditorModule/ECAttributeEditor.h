// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "ForwardDefines.h"
#include <QObject>

#include "Vector3D.h"
#include "CoreStringUtils.h"
#include "ComponentInterface.h"
#include "AssetInterface.h"
#include <map>

#include "MultiEditPropertyManager.h"
#include "MultiEditPropertyFactory.h"

class QtDoublePropertyManager;
class QtVariantPropertyManager;
class QtProperty;
class QtAbstractPropertyManager;
class QtAbstractEditorFactoryBase;
class QtAbstractPropertyBrowser;

class Color;

namespace Foundation
{
    class AttributeInterface;
    //class ComponentInterface;
    //typedef boost::shared_ptr<ComponentInterface> ComponentInterfacePtr;
    template<typename T> class Attribute;
    typedef std::vector<AttributeInterface*> AttributeVector;
}

namespace ECEditor
{
    //! ECAttributeEditorBase class.
    /*! Abstract base class for attribute editing. User can add editable components using the AddNewComponent method and the component is inculded
     *  inside the object's map. Note! ECAttributeEditor wont update the ui until UpdateEditorUI method is called.
     *  If attribute is edited outside of the editor, user need to inform the editor by calling a AttributeValueChanged method, witch will get a new attribute values from the
     *  AttributeInterface and update it's ui. If you are planing to add new attribute type to editor, you should take a look at ECAttributeEditor's template implementation code
     *  to see how other attribute types have been included into the editor.
     *  \todo Make this class struture more simple and rename those methods in a way that they will give a user better understanding what they are ment to do.
     *  \todo Remove QtAbstractPropertyBrowser pointer from the attribute editor, this means that manager and factory connections need to 
     *        be registered in elsewhere eg. inside the ECComponentEditor's addAttribute mehtod.
     *  \todo Remove component  weak pointers from the editor and only use attribute raw pointers for editing.
     *  This should make editor simpler to use. Note! Component editor need to make sure that ECAttributeEditor's attribute interfaces
     *  are always valid.
     *  \ingroup ECEditorModuleClient.
     */
    class ECAttributeEditorBase: public QObject
    {
        Q_OBJECT
    public:
        enum AttributeEditorState
        {
            Uninitialized,
            AttributeEdited,
            AttributeUpdated,
            WaitingForResponse
        };

        ECAttributeEditorBase(const QString &attributeName,
                                   QtAbstractPropertyBrowser *owner,
                                   Foundation::ComponentPtr component,
                                   QObject *parent = 0);

        ECAttributeEditorBase(const QString &attributeName,
                                   QtAbstractPropertyBrowser *owner,
                                   std::vector<Foundation::ComponentPtr> components,
                                   QObject *parent = 0);

        virtual ~ECAttributeEditorBase();

        //! Get attribute name.
        //! @return attribute type name.
        QString GetAttributeName() const { return attributeName_; }

        //! Get editor's root property.
        //! @return editor's root property pointer.
        QtProperty *GetProperty() const { return rootProperty_; }

        //! Check if this editor's manager contain this spesific property.
        //! @param property Property that we are looking for.
        bool ContainProperty(QtProperty *property) const;

        //! Add multiple components into the map. Note! editor wont update the editor's ui until the user
        //! has called UpdateEditorUI() method.
        //! @param components list of components that are included inside the map.
        void AddNewComponents(std::vector<Foundation::ComponentPtr>  components);

        //! Update editor's ui elements to fit new attribute values, if different component's attribute valeus differ from
        //! each other the editor begin to use multiedit mode and editor need to create new ui elements.
        void UpdateEditorUI();

        //! Get new attribute values and update them in browser window. 
        virtual void UpdateValue() = 0;

    public slots:
        //! update edtior ui when attribute value is changed.
        void AttributeValueChanged(const Foundation::AttributeInterface &attribute);
        
        //! Listens if any of editor's values has been changed and then for editor's attribute values need to be updated.
        void SendNewAttributeValue(QtProperty *property) 
        { 
            SendValue(property); 
        }

        //! Add new entity component into the map. Note! editor wont update the editor's ui until the user
        //! has called updateEditor() method, this way editor wont redraw it's elements until the user has inserted
        //! all the components that he want to edit.
        //! @param component New component that contains edited attribute.
        //! @param updateUi Do we want to update ui after we have added a new component to editor. Do not set true if you
        //! know that more same type of components are coming soon so we can avoid unnecessary repainting.
        void AddNewComponent(Foundation::ComponentPtr component, bool updateUi = true);

        //! Delete component from the editor's attributeMap object.
        //! @param component The component that we want to remove.
        void RemoveComponent(Foundation::ComponentInterface *component);

    private slots:
        //! Called when user has picked one of the multiselect values.
        //! @param property Property that we are editing.
        //! @param value new value that has been picked.
        void MultiEditValueSelected(const QtProperty *property, const QString &value) {ValueSelected(property, value);}

    signals:
        //! Attribute value has been changed by the editor.
        void AttributeChanged(const std::string &attributeName);

        //! When attribute editor is fully intialized this signal is emitted.
        //void AttributeEditorCreated(const QtProperty *property);

    protected:
        //! Multiedit value has been selected and it need to be type casted from string to it's original form using lexical_cast.
        virtual void ValueSelected(const QtProperty *property, const QString &value) = 0;

        //! Send new value to each component's attribute.
        virtual void SendValue(QtProperty *property) = 0;

        //! Initialize attribute editor's components.
        virtual void Initialize() = 0;
        //! PreInitialize should be called before the Initialize.
        void PreInitialize();
        //! Delete property manager and it's factory.
        void UnInitialize();

        //! Find the right attribute by using it's name and return it's pointer to user.
        //! @return If right attribute is found return it's pointer, if not return null.
        virtual Foundation::AttributeInterface *FindAttribute(Foundation::AttributeVector attributes);

        //! Check if all components contain right attribute value.
        //! @return return true if all attributes have a same value and false when not.
        virtual bool AttributesValueCheck() const;

        QtAbstractPropertyBrowser *owner_;
        QtAbstractPropertyManager *propertyMgr_;
        QtAbstractEditorFactoryBase *factory_;
        std::vector<QtAbstractPropertyManager*> optionalPropertyManagers_;
        std::vector<QtAbstractEditorFactoryBase*> optionalPropertyFactories_;
        QtProperty *rootProperty_;
        QString attributeName_;
        bool listenEditorChangedSignal_;
        bool componentIsSerializable_;

        typedef std::map<Foundation::ComponentWeakPtr, Foundation::AttributeInterface *> ECAttributeMap;
        ECAttributeMap attributeMap_;
        bool useMultiEditor_;
        //bool isInitialized_;
        AttributeEditorState editorState_;
    };

    //! ECAttributeEditor template class that initializes each attribute type's visual elements and handle those changes by using AttributeInterface.
    /*! To add support for a new attribute types you need to reimpement following methods:
     *   - Initialize: For intializing all ui elements for the editor. In this class the user need to choose right 
     *     PropertyManager and PropertyFactory that are reponssible for registering and creating all visual elements to the QtPropertyBrowser.
     *   - SendValue: Is a setter funtion for editor to AttributeInterface switch will send all user's
     *     made changes to actual object.
     *   - UpdateValue: Getter function between AttriubteInterface and Editor. Editor will ask attribute's value and
     *     set it to editor's ui element.
     *   - ValueSelected: This method is used for multiediting. When user has picked one of the multiedit options we need to 
     *     convert the string value to actual attribute value (usually this is done by using a boost's lexical_cast).
     */
    template<typename T> class ECAttributeEditor : public ECAttributeEditorBase
    {
    public:
        ECAttributeEditor(const QString &attributeName,
                          QtAbstractPropertyBrowser *owner,
                          Foundation::ComponentPtr component,
                          QObject *parent = 0):
            ECAttributeEditorBase(attributeName, owner, component, parent)
        {
            listenEditorChangedSignal_ = true;
        }

        ECAttributeEditor(const QString &attributeName,
                          QtAbstractPropertyBrowser *owner,
                          std::vector<Foundation::ComponentPtr> components,
                          QObject *parent = 0):
            ECAttributeEditorBase(attributeName, owner, components, parent)
        {
            listenEditorChangedSignal_ = true;
        }

        ~ECAttributeEditor()
        {
            
        }

        //! Get new entity components attribute value and change it on the editor widget.
        virtual void UpdateValue();

    private:
        virtual void ValueSelected(const QtProperty *property, const QString &value);
        virtual void SendValue(QtProperty *property);
        void Initialize();

        //! Sends a new value to each component and emit AttributeChanged signal.
        //! @param value_ new value that is sended into component's attribute.
        void SetValue(const T &value)
        {
            ECAttributeMap::iterator iter = attributeMap_.begin();
            while(iter != attributeMap_.end())
            {
                Foundation::ComponentWeakPtr component = iter->first;
                // If component has expired remove it from the map.
                if(component.expired())
                {
                    attributeMap_.erase(iter);
                    continue;
                }
                Foundation::Attribute<T> *attribute = dynamic_cast<Foundation::Attribute<T>*>(iter->second);
                if(attribute)
                {
                    //! if attribute is serializable then replicate those changes.
                    if(component.lock()->IsSerializable())
                    {
                        attribute->Set(value, AttributeChange::Local);
                        listenEditorChangedSignal_ = false;
                        attribute->GetOwner()->ComponentChanged(AttributeChange::Local);
                        listenEditorChangedSignal_ = true;
                    }
                    else
                    {
                        attribute->Set(value, AttributeChange::LocalOnly);
                        listenEditorChangedSignal_ = false;
                        attribute->GetOwner()->ComponentChanged(AttributeChange::LocalOnly);
                        listenEditorChangedSignal_ = true;
                    }
                }
                iter++;
            }
            editorState_ = AttributeEdited;
            //emit AttributeChanged(attributeName_.toStdString());
        }

        //! Create multiedit property manager and factory if listenEditorChangedSignal_ flag is rised.
        void InitializeMultiEditor()
        {
            ECAttributeEditorBase::PreInitialize();
            if(useMultiEditor_)
            {
                MultiEditPropertyManager *multiEditManager = new MultiEditPropertyManager(this);
                MultiEditPropertyFact *multiEditFactory = new MultiEditPropertyFact(this);
                propertyMgr_ = multiEditManager;
                factory_ = multiEditFactory;

                rootProperty_ = multiEditManager->addProperty(attributeName_);
                owner_->setFactoryForManager(multiEditManager, multiEditFactory);
                UpdateMultiEditorValue();
                QObject::connect(multiEditManager, SIGNAL(ValueChanged(const QtProperty *, const QString &)), this, SLOT(MultiEditValueSelected(const QtProperty *, const QString &)));
            }
        }

        //! Get each components atttribute value and convert it to string and put it send the string vector to
        //! multieditor manager.
        void UpdateMultiEditorValue()
        {
            if(useMultiEditor_ && componentIsSerializable_)
            {
                ECAttributeMap::iterator iter = attributeMap_.begin();
                QStringList stringList;
                MultiEditPropertyManager *testPropertyManager = dynamic_cast<MultiEditPropertyManager *>(propertyMgr_);
                while(iter != attributeMap_.end())
                {
                    if(rootProperty_ && iter->second)
                    {
                        Foundation::Attribute<T> *attribute = dynamic_cast<Foundation::Attribute<T>*>(iter->second);
                        if(!attribute)
                        {
                            iter++;
                            continue;
                        }

                        QString newValue(attribute->ToString().c_str());
                        if(!stringList.contains(newValue))
                            stringList << newValue;
                    }
                    iter++;
                }
                testPropertyManager->SetAttributeValues(rootProperty_, stringList);
            }
        }
    };

    template<> void ECAttributeEditor<Real>::UpdateValue();
    template<> void ECAttributeEditor<Real>::Initialize();
    template<> void ECAttributeEditor<Real>::SendValue(QtProperty *property);
    template<> void ECAttributeEditor<Real>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<int>::UpdateValue();
    template<> void ECAttributeEditor<int>::Initialize();
    template<> void ECAttributeEditor<int>::SendValue(QtProperty *property);
    template<> void ECAttributeEditor<int>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<bool>::UpdateValue();
    template<> void ECAttributeEditor<bool>::Initialize();
    template<> void ECAttributeEditor<bool>::SendValue(QtProperty *property);
    template<> void ECAttributeEditor<bool>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<Vector3df>::UpdateValue();
    template<> void ECAttributeEditor<Vector3df>::Initialize();
    template<> void ECAttributeEditor<Vector3df>::SendValue(QtProperty *property);
    template<> void ECAttributeEditor<Vector3df>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<Color>::UpdateValue();
    template<> void ECAttributeEditor<Color>::Initialize();
    template<> void ECAttributeEditor<Color>::SendValue(QtProperty *property);
    template<> void ECAttributeEditor<Color>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<std::string>::UpdateValue();
    template<> void ECAttributeEditor<std::string>::Initialize();
    template<> void ECAttributeEditor<std::string>::SendValue(QtProperty *property);
    template<> void ECAttributeEditor<std::string>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<QVariant>::UpdateValue();
    template<> void ECAttributeEditor<QVariant>::Initialize();
    template<> void ECAttributeEditor<QVariant>::SendValue(QtProperty *property);
    template<> void ECAttributeEditor<QVariant>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<Foundation::AssetReference>::UpdateValue();
    template<> void ECAttributeEditor<Foundation::AssetReference>::Initialize();
    template<> void ECAttributeEditor<Foundation::AssetReference>::SendValue(QtProperty *property);
    template<> void ECAttributeEditor<Foundation::AssetReference>::ValueSelected(const QtProperty *property, const QString &value);
}


#endif
