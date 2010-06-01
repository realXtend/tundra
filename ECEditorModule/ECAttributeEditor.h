// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "StableHeaders.h"
#include <QObject>
#include <QtDoublePropertyManager>
#include <QtVariantPropertyManager>
#include "AttributeInterface.h"

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
                                   QObject *parent = 0);

        virtual ~ECAttributeEditorInterface();

        //! Get attribute name.
        //! @return attribute type name.
        QString GetAttributeName() const { return attributeName_; }

        //! Get editor's root property.
        //! @return editor's root property pointer.
        QtProperty *GetProperty() const { return property_; }

        //! Get property manager ptr.
        //! @return property manager pointer.
        QtAbstractPropertyManager *GetPropertyManager()  const { return propertyMgr_; }

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

        QtAbstractPropertyBrowser *owner_;
        QtAbstractPropertyManager *propertyMgr_;
        QtAbstractEditorFactoryBase *factory_;
        QtProperty *property_;
        QString attributeName_;
        bool listenEditorChangedSignal_;
    };

    template<typename T> class ECAttributeEditor : public ECAttributeEditorInterface
    {
    public:
        ECAttributeEditor(const QString &attributeName,
                          QtAbstractPropertyBrowser *owner,
                          Foundation::AttributeInterface *attribute,
                          QObject *parent = 0):
            ECAttributeEditorInterface(attributeName, owner, parent)
        {
            attribute_ = dynamic_cast<Foundation::Attribute<T> *>(attribute);
            if(attribute)
            {
                Foundation::ComponentInterface *component = attribute->GetOwner();
                if(component)
                    QObject::connect(component, SIGNAL(OnChanged()), this, SLOT(AttributeValueChanged()));
                InitializeEditor();
                listenEditorChangedSignal_ = true;
            }
        }

        ~ECAttributeEditor()
        {
        }

        //! Send a new value to each component and emit AttributeChanged signal.
        //! @param value_ new value that is sended to component's attribute.
        void SetValue(T value)
        {
            if(attribute_)
            {
                if(attribute_->GetOwner()->IsSerializable())
                {
                    attribute_->Set(value, Foundation::ComponentInterface::Local);
                    listenEditorChangedSignal_ = false;
                    attribute_->GetOwner()->ComponentChanged(Foundation::ComponentInterface::Local);
                    listenEditorChangedSignal_ = true;
                }
                else
                {
                    attribute_->Set(value, Foundation::ComponentInterface::LocalOnly);
                    listenEditorChangedSignal_ = false;
                    attribute_->GetOwner()->ComponentChanged(Foundation::ComponentInterface::LocalOnly);
                    listenEditorChangedSignal_ = true;
                }
                emit AttributeChanged();
            }
        }

        //! Get component attribute value.
        //! @return attribute value.
        T GetValue() const
        {
            return attribute_->Get();
        }

        //! 
        virtual void UpdateEditorValue();

    private:
        virtual void SendNewValueToAttribute(QtProperty *property);
        virtual void InitializeEditor();

        Foundation::Attribute<T> *attribute_;
    };

    template<> void ECAttributeEditor<Real>::UpdateEditorValue();

}


#endif
