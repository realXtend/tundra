// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECComponentEditor.h"

#include "AttributeInterface.h"
#include "ECAttributeEditor.h"
#include "ComponentInterface.h"
#include "Transform.h"

#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtProperty>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    // static
    //! @todo Replace this more practical implementation where new attribute type support would be more pratical.
    //! Like somesort of factory that is ownd by ComponentManager.
    ECAttributeEditorBase *ECComponentEditor::CreateAttributeEditor(
        QtAbstractPropertyBrowser *browser,
        ECComponentEditor *editor,
        AttributeInterface &attribute)
    {
        ECAttributeEditorBase *attributeEditor = 0;
        if(dynamic_cast<const Attribute<float> *>(&attribute))
            attributeEditor = new ECAttributeEditor<float>(browser, &attribute, editor);
        else if(dynamic_cast<const Attribute<int> *>(&attribute))
            attributeEditor = new ECAttributeEditor<int>(browser, &attribute, editor);
        else if(dynamic_cast<const Attribute<Vector3df> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Vector3df>(browser, &attribute, editor);
        else if(dynamic_cast<const Attribute<Color> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Color>(browser, &attribute, editor);
        else if(dynamic_cast<const Attribute<QString> *>(&attribute))
            attributeEditor = new ECAttributeEditor<QString>(browser, &attribute, editor);
        else if(dynamic_cast<const Attribute<bool> *>(&attribute))
            attributeEditor = new ECAttributeEditor<bool>(browser, &attribute, editor);
        else if(dynamic_cast<const Attribute<QVariant> *>(&attribute))
            attributeEditor = new ECAttributeEditor<QVariant>(browser, &attribute, editor);
        else if(dynamic_cast<const Attribute<std::vector<QVariant> > *>(&attribute))
            attributeEditor = new ECAttributeEditor<std::vector<QVariant> >(browser, &attribute, editor);
        else if(dynamic_cast<const Attribute<Foundation::AssetReference> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Foundation::AssetReference>(browser, &attribute, editor);
        else if(dynamic_cast<const Attribute<Transform> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Transform>(browser, &attribute, editor);
        return attributeEditor;
    }

    ECComponentEditor::ECComponentEditor(Foundation::ComponentInterfacePtr component, QtAbstractPropertyBrowser *propertyBrowser):
        QObject(propertyBrowser),
        groupProperty_(0),
        groupPropertyManager_(0),
        propertyBrowser_(propertyBrowser)
    {
        typeName_ = component->TypeName();
        name_ = component->Name();
        InitializeEditor(component);
    }
    
    ECComponentEditor::~ECComponentEditor()
    {
        propertyBrowser_->unsetFactoryForManager(groupPropertyManager_);
        SAFE_DELETE(groupProperty_)
        SAFE_DELETE(groupPropertyManager_)
        while(!attributeEditors_.empty())
        {
            SAFE_DELETE(attributeEditors_.begin()->second)
            attributeEditors_.erase(attributeEditors_.begin());
        }
    }

    void ECComponentEditor::InitializeEditor(Foundation::ComponentInterfacePtr component)
    {
        if(!propertyBrowser_)
           return;

        groupPropertyManager_ = new QtGroupPropertyManager(this);
        if(groupPropertyManager_)
        {
            groupProperty_ = groupPropertyManager_->addProperty();
            AddNewComponent(component, true);
            CreateAttriubteEditors(component);
        }
        propertyBrowser_->addProperty(groupProperty_);
    }

    void ECComponentEditor::CreateAttriubteEditors(Foundation::ComponentInterfacePtr component)
    {
        AttributeVector attributes = component->GetAttributes();
        for(uint i = 0; i < attributes.size(); i++)
        {
            ECAttributeEditorBase *attributeEditor = ECComponentEditor::CreateAttributeEditor(propertyBrowser_, this, *attributes[i]);
            if(!attributeEditor)
                continue;
            attributeEditors_[attributes[i]->GetName()] = attributeEditor;
            attributeEditor->UpdateEditorUI();
            groupProperty_->setToolTip("Component type is " + component->TypeName());
            groupProperty_->addSubProperty(attributeEditor->GetProperty());

            QObject::connect(attributeEditor, SIGNAL(AttributeChanged(const std::string &)), this, SLOT(AttributeEditorUpdated(const std::string &)));
        }
    }

    void ECComponentEditor::UpdateGroupPropertyText()
    {
        if(!groupProperty_ || !components_.size())
            return;
        std::string componentName = typeName_.toStdString(); //\todo remove the back&forth string conversions XXX
        ReplaceSubstringInplace(componentName, "EC_", "");
        QString groupPropertyName = componentName.c_str();
        if(!name_.isEmpty())
            groupPropertyName += " (" + name_ + ") ";
        if(components_.size() > 1)
            groupPropertyName += QString(" (%1 components)").arg(components_.size());
        groupProperty_->setPropertyName(groupPropertyName);
    }

    bool ECComponentEditor::ContainProperty(QtProperty *property) const
    {
        AttributeEditorMap::const_iterator constIter = attributeEditors_.begin();
        while(constIter != attributeEditors_.end())
        {
            if(constIter->second->ContainProperty(property))
                return true;
            constIter++;
        }
        return false;
    }

    void ECComponentEditor::AddNewComponent(Foundation::ComponentInterfacePtr component, bool updateUi)
    {
        //! Check that component type is same as editor's typename (We only want to add same type of components to editor).
        if(component->TypeName() != typeName_)
            return;

        components_.insert(component);
        //! insert new component for each attribute editor.
        AttributeEditorMap::iterator iter = attributeEditors_.begin();
        while(iter != attributeEditors_.end())
        {
            AttributeInterface *attribute = component->GetAttribute(iter->second->GetAttributeName().toStdString());
            if(attribute)
                iter->second->AddNewAttribute(attribute);
            iter++;
        }
        QObject::connect(component.get(), SIGNAL(OnChanged()), this, SLOT(ComponentChanged()));
        UpdateGroupPropertyText();
    }

    void ECComponentEditor::RemoveComponent(Foundation::ComponentInterface *component)
    {
        if(!component)
            return;

        if(component->TypeName() != typeName_)
            return;

        ComponentSet::iterator iter = components_.begin();
        while(iter != components_.end())
        {
            Foundation::ComponentPtr componentPtr = (*iter).lock();
            if(componentPtr.get() == component)
            {
                AttributeEditorMap::iterator attributeIter = attributeEditors_.begin();
                while(attributeIter != attributeEditors_.end())
                {
                    AttributeInterface *attribute = componentPtr->GetAttribute(attributeIter->second->GetAttributeName().toStdString());
                    if(attribute)
                        attributeIter->second->RemoveAttribute(attribute);
                    attributeIter++;
                }
                components_.erase(iter);
                break;
            }
            iter++;
        }
        UpdateGroupPropertyText();
    }

    void ECComponentEditor::UpdateEditorUI()
    {
        AttributeEditorMap::iterator attributeIter = attributeEditors_.begin();
        while(attributeIter != attributeEditors_.end())
        {
            attributeIter->second->UpdateEditorUI();
            attributeIter++; 
        }
    }

    void ECComponentEditor::AttributeEditorUpdated(const std::string &attributeName)
    {
        AttributeEditorMap::iterator iter = attributeEditors_.begin();
        int index = 0;
        for(; iter != attributeEditors_.end(); iter++)
        {
            if(iter->first == attributeName)
            {
                QtProperty *newProperty = iter->second->GetProperty();
                QtProperty *afterProperty = 0;
                QList<QtProperty *> properties = groupProperty_->subProperties();
                if(properties.size() > 0 && properties.size() >= index)
                {
                    if(index > 0)
                        afterProperty = properties[index - 1];
                    else
                        afterProperty = properties[index];
                    groupProperty_->insertSubProperty(newProperty, afterProperty);
                    break;
                }
            }
            index++;
        }
    }

    void ECComponentEditor::ComponentChanged()
    {
        Foundation::ComponentInterface *component = dynamic_cast<Foundation::ComponentInterface *>(sender());
        if(!component)
            return;
        if(component->TypeName() != typeName_)
            return;

        AttributeVector attributes = component->GetAttributes();
        for(uint i = 0; i < attributes.size(); i++)
        {
            if(attributes[i]->IsDirty())
            {
                AttributeEditorMap::iterator iter = attributeEditors_.find(attributes[i]->GetName());
                if(iter != attributeEditors_.end())
                    iter->second->UpdateEditorUI();//AttributeValueChanged(*attributes[i]);
            }
        }
    }
}
