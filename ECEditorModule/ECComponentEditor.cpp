#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ECComponentEditor.h"
#include "AttributeInterface.h"
#include "ECAttributeEditor.h"

#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtProperty>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    // static
    ECAttributeEditorBase *ECComponentEditor::GetAttributeEditor(QtAbstractPropertyBrowser *browser, 
                                                                 ECComponentEditor *editor, 
                                                                 const Foundation::AttributeInterface &attribute, 
                                                                 Foundation::ComponentInterfacePtr component)
    {
        ECAttributeEditorBase *attributeEditor = 0;
        if(dynamic_cast<const Foundation::Attribute<Real> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Real>(attribute.GetName(), browser, component, editor);
        else if(dynamic_cast<const Foundation::Attribute<int> *>(&attribute))
            attributeEditor = new ECAttributeEditor<int>(attribute.GetName(), browser, component, editor);
        else if(dynamic_cast<const Foundation::Attribute<Vector3df> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Vector3df>(attribute.GetName(), browser, component, editor);
        else if(dynamic_cast<const Foundation::Attribute<Color> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Color>(attribute.GetName(), browser, component, editor);
        else if(dynamic_cast<const Foundation::Attribute<std::string> *>(&attribute))
            attributeEditor = new ECAttributeEditor<std::string>(attribute.GetName(), browser, component, editor);
        else if(dynamic_cast<const Foundation::Attribute<bool> *>(&attribute))
            attributeEditor = new ECAttributeEditor<bool>(attribute.GetName(), browser, component, editor);
        return attributeEditor;
    }

    ECComponentEditor::ECComponentEditor(Foundation::ComponentInterfacePtr component, const std::string &typeName, QtAbstractPropertyBrowser *propertyBrowser):
        typeName_(typeName),
        QObject(propertyBrowser),
        groupProperty_(0),
        groupPropertyManager_(0),
        propertyBrowser_(propertyBrowser)
    {
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

    void ECComponentEditor::InitializeEditor(Foundation::ComponentInterfacePtr component)//std::vector<Foundation::ComponentInterfacePtr> components)
    {
        if(propertyBrowser_)
        {
            groupPropertyManager_ = new QtGroupPropertyManager(this);
            if(groupPropertyManager_)
            {
                components_.insert(component);
                groupProperty_ = groupPropertyManager_->addProperty();
                CreateAttriubteEditors(component);
            }
            propertyBrowser_->addProperty(groupProperty_);
            UpdateGroupPropertyText();
        }
    }

    void ECComponentEditor::CreateAttriubteEditors(Foundation::ComponentInterfacePtr component)
    {
        Foundation::AttributeVector attributes = component->GetAttributes();
        for(uint i = 0; i < attributes.size(); i++)
        {
            ECAttributeEditorBase *attributeEditor = ECComponentEditor::GetAttributeEditor(propertyBrowser_, this, *attributes[i], component);
            if(!attributeEditor)
                continue;
            attributeEditors_[attributes[i]->GetName()] = attributeEditor;
            attributeEditor->UpdateEditorUI();
            groupProperty_->addSubProperty(attributeEditor->GetProperty());

            QObject::connect(attributeEditor, SIGNAL(AttributeChanged(const std::string &)), this, SIGNAL(AttributeChanged(const std::string &)));
            QObject::connect(attributeEditor, SIGNAL(AttributeChanged(const std::string &)), this, SLOT(AttributeEditorUpdated(const std::string &)));
            QObject::connect(this, SIGNAL(ComponentRemoved(Foundation::ComponentInterface *)), attributeEditor, SLOT(RemoveComponentFromEditor(Foundation::ComponentInterface *)));
        }
    }

    void ECComponentEditor::UpdateGroupPropertyText()
    {
        if(groupProperty_)
        {
            std::string componentName = typeName_;
            ReplaceSubstringInplace(componentName, "EC_", "");
            QString groupPropertyName = componentName.c_str();
            if(components_.size() > 1)
                groupPropertyName += QString(" (%1 components)").arg(components_.size());
            groupProperty_->setPropertyName(groupPropertyName);
        }
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

    void ECComponentEditor::AddNewComponent(Foundation::ComponentInterfacePtr component)
    {
        //! Check that component type is same as editor's typename (We only want to add same type of components to editor).
        if(component->TypeName() == typeName_)
        {
            components_.insert(component);
            AttributeEditorMap::iterator iter = attributeEditors_.begin();

            //! insert new component for each attribute editor.
            while(iter != attributeEditors_.end())
            {
                iter->second->AddNewComponent(component);
                iter->second->UpdateEditorUI();
                iter++;
            }
            UpdateGroupPropertyText();
        }
    }

    void ECComponentEditor::RemoveComponent(Foundation::ComponentInterfacePtr component)
    {
        // If component is already destoyed from the attribute browser no point to try to do it again.
        if(component.get())
        {
            if(component->TypeName() == typeName_)
            {
                ComponentSet::iterator iter = components_.begin();
                while(iter != components_.end())
                {
                    if(iter->expired())
                    {
                        // doesn't compile on gcc
                        // iter = components_.erase(iter);
                        continue;
                    }

                    Foundation::ComponentPtr componentPtr = (*iter).lock();
                    if(componentPtr.get() == component.get())
                    {
                        emit ComponentRemoved(componentPtr.get());
                        components_.erase(iter);
                        break;
                    }
                    iter++;
                }
                UpdateGroupPropertyText();
            }
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
                }
            }
            index++;
        }
    }
}
