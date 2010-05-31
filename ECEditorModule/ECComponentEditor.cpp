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
        else if(dynamic_cast<const Foundation::Attribute<Color> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Color>(attribute.GetName(), browser, component, editor);

        if(attributeEditor)
        {
            QObject::connect(attributeEditor, SIGNAL(AttributeChanged(const std::string &)), editor, SIGNAL(AttributeChanged(const std::string &)));
            QObject::connect(attributeEditor, SIGNAL(AttributeChanged(const std::string &)), editor, SLOT(AttributeEditorUpdated(const std::string &)));
        }
        return attributeEditor;
    }

    ECComponentEditor::ECComponentEditor(std::vector<Foundation::ComponentInterfacePtr> components, QtAbstractPropertyBrowser *propertyBrowser, QObject *parent):
        QObject(parent),
        groupProperty_(0),
        groupPropertyManager_(0),
        propertyBrowser_(propertyBrowser)
    {
        InitializeEditor(components);
    }
    
    ECComponentEditor::~ECComponentEditor()
    {
        SAFE_DELETE(groupProperty_)
        SAFE_DELETE(groupPropertyManager_)
        while(!attributeEditors_.empty())
        {
            SAFE_DELETE(attributeEditors_.begin()->second)
            attributeEditors_.erase(attributeEditors_.begin());
        }
    }

    void ECComponentEditor::InitializeEditor(std::vector<Foundation::ComponentInterfacePtr> components)
    {
        if(propertyBrowser_ || components.size() > 0)
        {
            groupPropertyManager_ = new QtGroupPropertyManager(this);
            if(groupPropertyManager_)
            {
                std::string groupText = std::string(components[0]->TypeName()); 
                if( components.size() > 1)
                    groupText += std::string(" (" + ::ToString<int>(components.size()) + " Components)");

                //Create all attribute types that are supported and put them into a map container.
                groupProperty_ = groupPropertyManager_->addProperty(groupText.c_str());
                if(groupProperty_)
                    CreateECAttributes(components[0]);

                // And tell the editor what componets have that attribute in use so that user can change their values.
                AttributeEditorMap::iterator iter = attributeEditors_.begin();
                while(iter != attributeEditors_.end())
                {
                    iter->second->AddNewComponents(components);
                    //iter->second->SetParentProperty(groupProperty_);
                    iter->second->UpdateEditorUI();
                    groupProperty_->addSubProperty(iter->second->GetProperty());
                    iter++;
                }
            }
            propertyBrowser_->addProperty(groupProperty_);
        }
        // If propertyBrowser is not valid or there isn't any components to edit no point to keep the editor alive.
        else
        {
            deleteLater();
        }
    }

    void ECComponentEditor::CreateECAttributes(Foundation::ComponentInterfacePtr components)
    {
        Foundation::AttributeVector attributes = components->GetAttributes();
        for(uint i = 0; i < attributes.size(); i++)
        {
            ECAttributeEditorBase *attributeEditor = ECComponentEditor::GetAttributeEditor(propertyBrowser_, this, *attributes[i], components);
            if(!attributeEditor)
                continue;
            attributeEditors_[attributes[i]->GetName()] = attributeEditor;
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