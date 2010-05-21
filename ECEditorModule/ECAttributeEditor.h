// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "ForwardDefines.h"
#include <QObject>
//#include <QtDoublePropertyManager>
//#include <QtVariantPropertyManager>
#include "AttributeInterface.h"
#include <map>

class QtDoublePropertyManager;
class QtVariantPropertyManager;
class QtProperty;
class QtAbstractPropertyManager;
class QtAbstractEditorFactoryBase;
class QtAbstractPropertyBrowser;

namespace Foundation
{
    class AttributeInterface;
    template<typename T> class Attribute;
}

namespace ECEditor
{
    class ECAttributeEditorInterface : public QObject
    {
        Q_OBJECT
    public:
        ECAttributeEditorInterface(const QString &attributeName,
                                   QtAbstractPropertyBrowser *owner,
                                   Foundation::ComponentPtr component,
                                   QObject *parent = 0);

        ECAttributeEditorInterface(const QString &attributeName,
                                   QtAbstractPropertyBrowser *owner,
                                   std::vector<Foundation::ComponentPtr> components,
                                   QObject *parent = 0);

        virtual ~ECAttributeEditorInterface();

        //! Get attribute name.
        //! @return attribute type name.
        QString GetAttributeName() const { return attributeName_; }

        //! Get editor's root property.
        //! @return editor's root property pointer.
        const QtProperty *GetProperty() const { return property_; }

        //! Return number of entity componets have been attached to this editor.
        //! @return Number of entity components in this editor.
        int NumberOfComponents() const { return attributeMap_.size();}

        //! Get property manager ptr.
        //! @return property manager pointer.
        QtAbstractPropertyManager *GetPropertyManager()  const { return propertyMgr_; }

        void AddNewComponent(Foundation::ComponentPtr component);

        //! Get new attribute values and update them in browser window.
        virtual void UpdateEditorValue() = 0;

    public slots:
        //! Listens if any of component's attributes has changed.
        void AttributeValueChanged() { UpdateEditorValue(); }
        
        //! Listens if any of editor's values has been changed and changed and component's attributes need to be updated.
        void SendNewAttributeValue(QtProperty *property) { SendNewValueToAttribute(property); }

    signals:
        //! Signals when attribute values has been updated. This is used to inform the editor window
        //! when the xml line-editor need to be updated.
        void AttributeChanged();

    protected:
        //! Sends a new value to each component attribute.
        virtual void SendNewValueToAttribute(QtProperty *property) = 0;

        //! Initialize attribute editor's components.
        virtual void InitializeEditor() = 0;
        virtual void UninitializeEditor();

        virtual Foundation::AttributeInterface *FindAttribute(Foundation::AttributeVector attributes);

        //! Check if all attributes contain the same value.
        //! @return return true if all attributes have a same value and false when not.
        virtual bool AttributesValueCheck() const;

        QtAbstractPropertyBrowser *owner_;
        QtAbstractPropertyManager *propertyMgr_;
        QtAbstractEditorFactoryBase *factory_;
        QtProperty *property_;
        QString attributeName_;
        bool listenEditorChangedSignal_;
        typedef std::map<Foundation::ComponentWeakPtr, Foundation::AttributeInterface *> ECAttributeMap;
        ECAttributeMap attributeMap_;
        bool useMultiEditor_;
    };

    template<typename T> class ECAttributeEditor : public ECAttributeEditorInterface
    {
    public:
        ECAttributeEditor(const QString &attributeName,
                          QtAbstractPropertyBrowser *owner,
                          Foundation::ComponentPtr component,
                          QObject *parent = 0):
            ECAttributeEditorInterface(attributeName, owner, component, parent)
        {
            InitializeEditor();
            listenEditorChangedSignal_ = true;
        }

        ECAttributeEditor(const QString &attributeName,
                          QtAbstractPropertyBrowser *owner,
                          std::vector<Foundation::ComponentPtr> components,
                          QObject *parent = 0):
            ECAttributeEditorInterface(attributeName, owner, components, parent)
        {
            InitializeEditor();
            listenEditorChangedSignal_ = true;
        }

        ~ECAttributeEditor()
        {
            
        }

        //! Get new entity components attribute value and change it on the editor widget.
        virtual void UpdateEditorValue();

    private:
        virtual void SendNewValueToAttribute(QtProperty *property);
        virtual void InitializeEditor();

        //! Send a new value to each component and emit a AttributeChanged signal.
        //! @param value_ new value that is sended to component's attribute.
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

    template<> void ECAttributeEditor<Real>::UpdateEditorValue();
    template<> void ECAttributeEditor<Real>::InitializeEditor();
    template<> void ECAttributeEditor<Real>::SendNewValueToAttribute(QtProperty *property);
}


#endif
