// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "ForwardDefines.h"
#include <QObject>

#include "AttributeInterface.h"
#include "Vector3D.h"
#include "CoreStringUtils.h"
#include <map>

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
    //! Abstract base class for attribute editing. User can add editable components using a AddNewComponent interface and the component is inculded
    //! inside the object's map. Note! ECAttributeEditor wont update the ui until UpdateEditorUI method is called.
    //! If attribute is edited outside of the editor, user need to inform the editor by calling a AttributeValueChanged method, witch will get new attribute values from the
    //! AttributeInterface and update it's ui. If you are planing to add new attribute editor types you should take a look at ECAttributeEditor template class to see
    //! how the other attribute types are included into the editor.
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

        //! Return number of entity componets have been attached to this editor.
        //! @return Number of entity components in this editor.
        int NumberOfComponents() const { return attributeMap_.size();}

        //! Get property manager ptr.
        //! @return property manager pointer.
        QtAbstractPropertyManager *GetPropertyManager()  const { return propertyMgr_; }

        //! Add new entity component into the map. Note! editor wont update the editor's ui until the user
        //! has called updateEditor() method.
        //! @param component new component that contains edited attribute.
        void AddNewComponent(Foundation::ComponentPtr component);

        //! Add multiple components into the map. Note! editor wont update the editor's ui until the user
        //! has called updateEditor() method.
        //! @param components list of components that are included inside the map.
        void AddNewComponents(std::vector<Foundation::ComponentPtr>  components);

        //! Reintialize the editor's ui elements.
        void UpdateEditorUI();

        //! Add this editors root protety as other property's child.
        //void SetParentProperty(QtProperty *parent){ parentProperty_ = parent; }
        //! Get editor's root property (might contain other children).
        //QtProperty *GetParentProperty() const { return parentProperty_; }

        //! Get new attribute values and update them in browser window.
        virtual void UpdateEditorValue() = 0;

    public slots:
        //! update edtior ui when attribute value is changed.
        void AttributeValueChanged() 
        {
            UpdateEditorValue();
        }
        
        //! Listens if any of editor's values has been changed and changed and component's attributes need to be updated.
        void SendNewAttributeValue(QtProperty *property) { SendNewValueToAttribute(property); }

    private slots:
        //! If multiselect is in use and user have pick one of the value for all attributes this one is called.
        void MultiSelectValueSelected(const QtProperty *property, const QString &value) {ValueSelected(property, value);}

    signals:
        //! Attribute value has been changed by the editor.
        void AttributeChanged(const std::string &attributeName);

    protected:
        //! Multiedit value has been selected and it need to be type casted from string to it's original form using lexical_cast.
        virtual void ValueSelected(const QtProperty *property, const QString &value) = 0;
        //! Sends a new value to each component attribute.
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

        QtAbstractPropertyBrowser *owner_;
        QtAbstractPropertyManager *propertyMgr_;
        QtAbstractEditorFactoryBase *factory_;
        QtProperty *rootProperty_;
        //QtProperty *parentProperty_;
        QString attributeName_;
        bool listenEditorChangedSignal_;
        bool componentIsSerializable_;
        typedef std::map<Foundation::ComponentWeakPtr, Foundation::AttributeInterface *> ECAttributeMap;
        ECAttributeMap attributeMap_;
        bool useMultiEditor_;
    };

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

        //! Create multiedit property manager and factory if listenEditorChangedSignal_ flag is rised.
        void InitializeMultiEditor();
        //! Get each components atttribute value and convert it to string and put it send the string vector to
        //! multieditor manager.
        void UpdateMultiEditorValue();

        //! In some cases this method need to be overrided when we are converting values that can't be lexical_casted.
        //! More information about lexical_cast can be found at boost documentation.
        //! @param value value that we want to convert with the lexical_cast.
        /*T StringToValue(const QString &value)
        {
            T returnValue = ParseString<T>(value.toStdString());
            return returnValue;
        }*/

        //! Sends a new value to each component and emit a AttributeChanged signal.
        //! @param value_ new value that is sended into component's attribute.
        void SetValue(const T &value)
        {
            ECAttributeMap::iterator iter = attributeMap_.begin();
            while(iter != attributeMap_.end())
            {
                Foundation::ComponentWeakPtr component = iter->first;
                if(component.expired())
                    continue;
                Foundation::Attribute<T> *attribute = dynamic_cast<Foundation::Attribute<T>*>(iter->second);
                if(attribute)
                {
                    //! if attribute is serializable then replicate those changes.
                    if(component.lock()->IsSerializable())
                    {
                        attribute->Set(value, Foundation::ComponentInterface::Local);
                        listenEditorChangedSignal_ = false;
                        attribute->GetOwner()->ComponentChanged(Foundation::ComponentInterface::Local);
                        listenEditorChangedSignal_ = true;
                    }
                    else
                    {
                        attribute->Set(value, Foundation::ComponentInterface::LocalOnly);
                        listenEditorChangedSignal_ = false;
                        attribute->GetOwner()->ComponentChanged(Foundation::ComponentInterface::LocalOnly);
                        listenEditorChangedSignal_ = true;
                    }
                }
                iter++;
            }
            emit AttributeChanged(attributeName_.toStdString());
        }
    };

    //template<typename T> void ECAttributeEditor<T>::ValueSelected(const QtProperty *property, const QString &value);
    template<typename T> void ECAttributeEditor<T>::InitializeMultiEditor();
    template<typename T> void ECAttributeEditor<T>::UpdateMultiEditorValue();

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
