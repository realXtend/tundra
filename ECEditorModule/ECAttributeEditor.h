// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "ForwardDefines.h"
#include <QObject>

#include "AttributeInterface.h"
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
    class ECComponentEditor;

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

        //! Trying to find the right attribute type by doing a dynamic cast and if object is succefully casted 
        //! it will create a new ECAttributeEditor object and return it's pointer to user.
        //! @return return attribute pointer if attribute type is supported if not return null pointer.
        static ECAttributeEditorBase *GetAttributeEditor( QtAbstractPropertyBrowser *browser, 
                                                          ECComponentEditor *editor, 
                                                          const Foundation::AttributeInterface &attribute,
                                                          Foundation::ComponentInterfacePtr component );

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

        void SetParentProperty(QtProperty *parent){ parentProperty_ = parent; }
        QtProperty *GetParentProperty() const { return parentProperty_; }

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
        //! Signals when attribute values has been updated. This is used to inform the editor window
        //! when the xml line-editor need to be updated.
        void AttributeChanged();

    protected:
        virtual void ValueSelected(const QtProperty *property, const QString &value) = 0;
        //! Sends a new value to each component attribute.
        virtual void SendNewValueToAttribute(QtProperty *property) = 0;

        //! Initialize attribute editor's components.
        virtual void InitializeEditor() = 0;
        //! PreInitialize should be called before the InitializeEditor.
        void PreInitializeEditor();
        void UninitializeEditor();

        virtual Foundation::AttributeInterface *FindAttribute(Foundation::AttributeVector attributes);

        //! Check if all components contain same attribute value.
        //! @return return true if all attributes have a same value and false when not.
        virtual bool AttributesValueCheck() const;

        QtAbstractPropertyBrowser *owner_;
        QtAbstractPropertyManager *propertyMgr_;
        QtAbstractEditorFactoryBase *factory_;
        QtProperty *rootProperty_;
        QtProperty *parentProperty_;
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
            //InitializeEditor();
            listenEditorChangedSignal_ = true;
        }

        ECAttributeEditor(const QString &attributeName,
                          QtAbstractPropertyBrowser *owner,
                          std::vector<Foundation::ComponentPtr> components,
                          QObject *parent = 0):
            ECAttributeEditorBase(attributeName, owner, components, parent)
        {
            //InitializeEditor();
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

        virtual void InitializeMultiEditor();
        void UpdateMultiEditorValue();

        //! In some cases this method need to be overrided when we are converting values that can't be lexical_casted.
        //! More information about lexical_cast can be found at boost documentation.
        //! @param value value that we want to convert with the lexical_cast.
        T StringToValue(const QString &value)
        {
            T returnValue = ParseString<T>(value.toStdString());
            return returnValue;
        }

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
                        attribute->Set(value, Foundation::Local);
                        listenEditorChangedSignal_ = false;
                        attribute->GetOwner()->ComponentChanged(Foundation::Local);
                        listenEditorChangedSignal_ = true;
                    }
                    else
                    {
                        attribute->Set(value, Foundation::LocalOnly);
                        listenEditorChangedSignal_ = false;
                        attribute->GetOwner()->ComponentChanged(Foundation::LocalOnly);
                        listenEditorChangedSignal_ = true;
                    }
                }
                iter++;
            }
            emit AttributeChanged();
        }
    };

    template<typename T> void ECAttributeEditor<T>::ValueSelected(const QtProperty *property, const QString &value);
    template<typename T> void ECAttributeEditor<T>::InitializeMultiEditor();
    template<typename T> void ECAttributeEditor<T>::UpdateMultiEditorValue();

    template<> void ECAttributeEditor<Real>::UpdateEditorValue();
    template<> void ECAttributeEditor<Real>::InitializeEditor();
    template<> void ECAttributeEditor<Real>::SendNewValueToAttribute(QtProperty *property);
    template<> void ECAttributeEditor<Real>::ValueSelected(const QtProperty *property, const QString &value);

    template<> void ECAttributeEditor<Color>::UpdateEditorValue();
    template<> void ECAttributeEditor<Color>::InitializeEditor();
    template<> void ECAttributeEditor<Color>::SendNewValueToAttribute(QtProperty *property);
    template<> void ECAttributeEditor<Color>::ValueSelected(const QtProperty *property, const QString &value);
}


#endif
