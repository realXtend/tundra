// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "ForwardDefines.h"
#include <QObject>

#include "AttributeInterface.h"
#include "ComponentInterface.h"
#include "Vector3D.h"
#include "CoreStringUtils.h"
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
    class ComponentInterface;
    typedef boost::shared_ptr<ComponentInterface> ComponentInterfacePtr;
    template<typename T> class Attribute;
}

namespace ECEditor
{
    //! ECAttributeEditorBase class.
    /*! Abstract base class for attribute editing. User can add editable components using the AddNewComponent method and the component is inculded
     *  inside the object's map. Note! ECAttributeEditor wont update the ui until UpdateEditorUI method is called.
     *  If attribute is edited outside of the editor, user need to inform the editor by calling a AttributeValueChanged method, witch will get a new attribute values from the
     *  AttributeInterface and update it's ui. If you are planing to add new attribute type to editor, you should take a look at ECAttributeEditor's template implementation code
     * to see how other attribute types have been included into the editor.
     * Todo! Make this class struture more simple and rename those methods in a way so that they give a user a better picture what they are ment to do.
     * Todo! Remove QtAbstractPropertyBrowser pointer from the attribute editor, this means that manager and factory need to be registered to PropertyBrowser elsewhere.
     * \ingroup ECEditorModuleClient.
     */
    class ECAttributeEditorBase: public QObject
    {
        Q_OBJECT
    public:
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

        //! Check if this eidtor's manager contain this spesific property.
        //! @param property Property that we are looking for.
        bool ContainProperty(QtProperty *property) const;

        //! Add multiple components into the map. Note! editor wont update the editor's ui until the user
        //! has called updateEditor() method.
        //! @param components list of components that are included inside the map.
        void AddNewComponents(std::vector<Foundation::ComponentPtr>  components);

        //! Reintialize the editor's ui elements.
        void UpdateEditorUI();

        //! Get new attribute values and update them in browser window. 
        virtual void UpdateEditorValue() = 0;

    public slots:
        //! update edtior ui when attribute value is changed.
        void AttributeValueChanged() 
        {
            UpdateEditorUI();
            //UpdateEditorValue();
        }
        
        //! Listens if any of editor's values has been changed and then for it's attributes need to be updated.
        void SendNewAttributeValue(QtProperty *property) 
        { 
            SendNewValueToAttribute(property); 
        }

        //! Add new entity component into the map. Note! editor wont update the editor's ui until the user
        //! has called updateEditor() method, this way editor wont redraw it's elements until the user has inserted
        //! all the components that he want to edit.
        //! @param component new component that contains edited attribute.
        void AddNewComponent(Foundation::ComponentPtr component);

        //! Delete component from the editor's attributeMap object.
        //! @param component The component that we want to remove.
        void RemoveComponentFromEditor(Foundation::ComponentInterface *component);

    private slots:
        //! Called when user has picked one of the multiselect values.
        //! @param property Property that we are editing.
        //! @param value new value that has been picked.
        void MultiEditValueSelected(const QtProperty *property, const QString &value) {ValueSelected(property, value);}

    signals:
        //! Attribute value has been changed by the editor.
        void AttributeChanged(const std::string &attributeName);

        //! When attribute editor is fully intialized this signal is emitted.
        void AttributeEditorCreated(const QtProperty *property);

    protected:
        //! Multiedit value has been selected and it need to be type casted from string to it's original form using lexical_cast.
        virtual void ValueSelected(const QtProperty *property, const QString &value) = 0;

        //! Send new value to each component's attribute.
        virtual void SendNewValueToAttribute(QtProperty *property) = 0;

        //! Initialize attribute editor's components.
        virtual void InitializeEditor() = 0;
        //! PreInitialize should be called before the InitializeEditor.
        void PreInitializeEditor();
        //! Delete property manager and it's factory.
        void UninitializeEditor();

        //! Find the right attribute by using it's name and return it's pointer to user.
        //! @return If right attribute is found return it's pointer, if not return null.
        virtual Foundation::AttributeInterface *FindAttribute(Foundation::AttributeVector attributes);

        //! Check if all components contain right attribute value.
        //! @return return true if all attributes have a same value and false when not.
        virtual bool AttributesValueCheck() const;

        //! TODO! remove owner and add manager and factory to property manager to property browser by using signals and slots.
        QtAbstractPropertyBrowser *owner_;
        QtAbstractPropertyManager *propertyMgr_;
        QtAbstractEditorFactoryBase *factory_;
        QtProperty *rootProperty_;
        QString attributeName_;
        bool listenEditorChangedSignal_;
        bool componentIsSerializable_;
        typedef std::map<Foundation::ComponentWeakPtr, Foundation::AttributeInterface *> ECAttributeMap;
        ECAttributeMap attributeMap_;
        bool useMultiEditor_;
        bool isInitialized_;
    };

    //! ECAttributeEditor template class that initializes each attribute type that we want to support in QtPropertyBrowser.
    /*! 
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
        virtual void UpdateEditorValue();

    private:
        virtual void ValueSelected(const QtProperty *property, const QString &value);
        virtual void SendNewValueToAttribute(QtProperty *property);
        void InitializeEditor();

        //! Sends a new value to each component and emit a AttributeChanged signal.
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
            emit AttributeChanged(attributeName_.toStdString());
        }

        //! Create multiedit property manager and factory if listenEditorChangedSignal_ flag is rised.

        void InitializeMultiEditor()
        {
            ECAttributeEditorBase::PreInitializeEditor();
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

    template<> void ECAttributeEditor<Real>::UpdateEditorValue();
    template<> void ECAttributeEditor<Real>::InitializeEditor();
    template<> void ECAttributeEditor<Real>::SendNewValueToAttribute(QtProperty *property);
    template<> void ECAttributeEditor<Real>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<int>::UpdateEditorValue();
    template<> void ECAttributeEditor<int>::InitializeEditor();
    template<> void ECAttributeEditor<int>::SendNewValueToAttribute(QtProperty *property);
    template<> void ECAttributeEditor<int>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<bool>::UpdateEditorValue();
    template<> void ECAttributeEditor<bool>::InitializeEditor();
    template<> void ECAttributeEditor<bool>::SendNewValueToAttribute(QtProperty *property);
    template<> void ECAttributeEditor<bool>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<Vector3df>::UpdateEditorValue();
    template<> void ECAttributeEditor<Vector3df>::InitializeEditor();
    template<> void ECAttributeEditor<Vector3df>::SendNewValueToAttribute(QtProperty *property);
    template<> void ECAttributeEditor<Vector3df>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<Color>::UpdateEditorValue();
    template<> void ECAttributeEditor<Color>::InitializeEditor();
    template<> void ECAttributeEditor<Color>::SendNewValueToAttribute(QtProperty *property);
    template<> void ECAttributeEditor<Color>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<std::string>::UpdateEditorValue();
    template<> void ECAttributeEditor<std::string>::InitializeEditor();
    template<> void ECAttributeEditor<std::string>::SendNewValueToAttribute(QtProperty *property);
    template<> void ECAttributeEditor<std::string>::ValueSelected(const QtProperty *property, const QString &value);
}


#endif
