#include "StableHeaders.h"
#include "AttributeBrowser.h"
#include "AttributeInterface.h"
#include "ECAttributeEditor.h"

#include <QtAbstractEditorFactoryBase>
#include <QtTreePropertyBrowser>
#include <QLayout>

namespace ECEditor
{
    AttributeBrowser::AttributeBrowser(QWidget *parent): QWidget(parent),
        propertyBrowser_(0)
    {
        InitializeEditor();
    }

    AttributeBrowser::~AttributeBrowser()
    {
    }

    void AttributeBrowser::AddEntityComponent(Foundation::ComponentInterfacePtr entityComponent)
    {
        QString name = QString(entityComponent.get()->TypeName().c_str());
        EntityComponentMap::iterator iter = SelectedEntityComponents_.find(name);
        if(iter == SelectedEntityComponents_.end())
        {
            SelectedEntityComponents_[name] = Foundation::ComponentWeakPtr(entityComponent);
            RefreshAttributeComponents();
        }
    }

    void AttributeBrowser::RemoveEntityComponent(Foundation::ComponentInterfacePtr entityComponent)
    {
        QString name = QString(entityComponent.get()->Name().c_str());
        EntityComponentMap::iterator iter = SelectedEntityComponents_.find(name);
        if(iter != SelectedEntityComponents_.end())
            SelectedEntityComponents_.erase(iter);
    }

    void AttributeBrowser::RefreshAttributeComponents()
    {
        EntityComponentMap::iterator iter = SelectedEntityComponents_.begin();
        while(iter != SelectedEntityComponents_.end())
        {
            if(iter->second.expired())
                continue;
            
            Foundation::ComponentInterfacePtr componentPtr = iter->second.lock();
            Foundation::AttributeVector attributes = componentPtr->GetAttributes();
            for(uint i = 0; i < attributes.size(); i++)
            {
                AddNewAttribute(attributes[i]);
            }
            iter++;
        }
    }

    void AttributeBrowser::ClearBrowser()
    {
        propertyBrowser_->clear();
        while(!SelectedEntityComponents_.empty())
            SelectedEntityComponents_.erase(SelectedEntityComponents_.begin());

        while(!attributes_.empty())
        {
            SAFE_DELETE(attributes_.begin()->second)
            attributes_.erase(attributes_.begin());
        }
    }

    void AttributeBrowser::InitializeEditor()
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);

        propertyBrowser_ = new QtTreePropertyBrowser(this);
        propertyBrowser_->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
        layout->addWidget(propertyBrowser_);
    }

    void AttributeBrowser::AddNewAttribute(Foundation::AttributeInterface *attribute)
    {
        AttributeEditorMap::iterator iter = attributes_.find(attribute->GetName());
        if(iter == attributes_.end())
        {
            ECAttributeEditorInterface *newEditor = CreateAttributeEditor(*attribute);
            if(newEditor)
                attributes_[newEditor->GetAttributeName()] = newEditor;
        }
        else
        {

            //todo! Set this attribute to multiedit mode cause more than one entity component has the same attribute type in use.
        }
    }

    ECAttributeEditorInterface *AttributeBrowser::CreateAttributeEditor(Foundation::AttributeInterface &attribute)
    {
        ECAttributeEditorInterface *attributeEditor = 0;
        // Todo! Organize those dynamic casts in a such order that we first check those attribute types that are most commonly used.
        if(dynamic_cast<const Foundation::Attribute<Real> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Real>(attribute.GetName(), propertyBrowser_, &attribute, this);
        else if(dynamic_cast<const Foundation::Attribute<int> *>(&attribute))
            attributeEditor = new ECAttributeEditor<int>(attribute.GetName(), propertyBrowser_, &attribute, this);
        else if(dynamic_cast<const Foundation::Attribute<bool> *>(&attribute))
            attributeEditor = new ECAttributeEditor<bool>(attribute.GetName(), propertyBrowser_, &attribute, this);
        else if(dynamic_cast<const Foundation::Attribute<Color> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Color>(attribute.GetName(), propertyBrowser_, &attribute, this);
        else if(dynamic_cast<const Foundation::Attribute<Vector3df> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Vector3df>(attribute.GetName(), propertyBrowser_, &attribute, this);

        if(attributeEditor)
            QObject::connect(attributeEditor, SIGNAL(AttributeChanged()), this, SIGNAL(AttributesChanged()));
        return attributeEditor;
    }
}