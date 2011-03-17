// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECComponentEditor.h"
#include "ECEditorModule.h"

#include "IAttribute.h"
#include "ECAttributeEditor.h"
#include "IComponent.h"
#include "Transform.h"
#include "AssetReference.h"
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("ECAttributeEditorBase")

#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtProperty>

#include "MemoryLeakCheck.h"

// static
ECAttributeEditorBase *ECComponentEditor::CreateAttributeEditor(
    QtAbstractPropertyBrowser *browser,
    ECComponentEditor *editor,
    ComponentPtr component,
    const QString &name,
    const QString &type)
{
    ECAttributeEditorBase *attributeEditor = 0;
    if(type == "real")
        attributeEditor = new ECAttributeEditor<float>(browser, component, name, type, editor);
    else if(type == "int")
        attributeEditor = new ECAttributeEditor<int>(browser, component, name, type, editor);
    else if(type == "vector3df")
        attributeEditor = new ECAttributeEditor<Vector3df>(browser, component, name, type, editor);
    else if(type == "color")
        attributeEditor = new ECAttributeEditor<Color>(browser, component, name, type, editor);
    else if(type == "string")
        attributeEditor = new ECAttributeEditor<QString>(browser, component, name, type, editor);
    else if(type == "bool")
        attributeEditor = new ECAttributeEditor<bool>(browser, component, name, type, editor);
    else if(type == "qvariant")
        attributeEditor = new ECAttributeEditor<QVariant>(browser, component, name, type, editor);
    else if(type == "qvariantlist")
        attributeEditor = new ECAttributeEditor<QVariantList>(browser, component, name, type, editor);
    else if(type == "assetreference")
        attributeEditor = new ECAttributeEditor<AssetReference>(browser, component, name, type, editor);
    else if(type == "assetreferencelist")
        attributeEditor = new ECAttributeEditor<AssetReferenceList>(browser, component, name, type, editor);
    else if(type == "transform")
        attributeEditor = new ECAttributeEditor<Transform>(browser, component, name, type, editor);
    else
        LogError("Unknown attribute type " + type.toStdString() + " for ECAttributeEditorBase creation.");

    return attributeEditor;
}

ECComponentEditor::ECComponentEditor(ComponentPtr component, QtAbstractPropertyBrowser *propertyBrowser):
    QObject(propertyBrowser),
    groupProperty_(0),
    groupPropertyManager_(0),
    propertyBrowser_(propertyBrowser)
{
    assert(component);
    typeName_ = component->TypeName();
    name_ = component->Name();

    assert(propertyBrowser_);
    if(!propertyBrowser_)
       return;

    groupPropertyManager_ = new QtGroupPropertyManager(this);
    if(groupPropertyManager_)
    {
        groupProperty_ = groupPropertyManager_->addProperty();
        CreateAttributeEditors(component);
        AddNewComponent(component);
    }

    propertyBrowser_->addProperty(groupProperty_);
}

ECComponentEditor::~ECComponentEditor()
{
    propertyBrowser_->unsetFactoryForManager(groupPropertyManager_);
    SAFE_DELETE(groupProperty_)
    SAFE_DELETE(groupPropertyManager_)
    while(!attributeEditors_.empty())
    {
        SAFE_DELETE(attributeEditors_.begin().value())
        attributeEditors_.erase(attributeEditors_.begin());
    }
}

void ECComponentEditor::CreateAttributeEditors(ComponentPtr component)
{
    AttributeVector attributes = component->GetAttributes();
    for(uint i = 0; i < attributes.size(); i++)
    {
        // Check metadata if this attribute is intended to be shown in designer/editor ui
        if (attributes[i]->HasMetadata())
            if (!attributes[i]->GetMetadata()->designable)
                continue;

        ECAttributeEditorBase *attributeEditor = ECComponentEditor::CreateAttributeEditor(propertyBrowser_, this,
            component, QString(attributes[i]->GetNameString().c_str()), QString(attributes[i]->TypeName().c_str()));
        if (!attributeEditor)
            continue;

        attributeEditors_[attributes[i]->GetName()] = attributeEditor;
        groupProperty_->setToolTip("Component type is " + component->TypeName());
        groupProperty_->addSubProperty(attributeEditor->GetProperty()); 
        connect(attributeEditor, SIGNAL(EditorChanged(const QString &)), this, SLOT(OnEditorChanged(const QString &)));
    }
}

void ECComponentEditor::UpdateGroupPropertyText()
{
    if(!groupProperty_ || !components_.size())
        return;
    QString componentName = typeName_;
    componentName.replace("EC_", "");
    QString groupPropertyName = componentName;
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
        if(constIter.value()->ContainsProperty(property))
            return true;
        constIter++;
    }
    return false;
}

void ECComponentEditor::AddNewComponent(ComponentPtr component)
{
    PROFILE(ECComponentEditor_AddNewComponent);
    //! Check that component type is same as editor's typename (We only want to add same type of components to editor).
    if(component->TypeName() != typeName_)
        return;

    components_.push_back(ComponentWeakPtr(component));
    //! insert new component to each attribute editor.
    AttributeEditorMap::iterator iter = attributeEditors_.begin();
    while(iter != attributeEditors_.end())
    {
        IAttribute *attribute = component->GetAttribute(iter.value()->GetAttributeName());
        if(attribute)
            iter.value()->AddComponent(component);
        iter++;
    }
    UpdateGroupPropertyText();
}

void ECComponentEditor::RemoveComponent(ComponentPtr component)
{
    if(!component)
        return;

    if(component->TypeName() != typeName_)
        return;

    ComponentSet::iterator iter = components_.begin();
    while(iter != components_.end())
    {
        ComponentPtr comp_ptr = (*iter).lock();
        if(comp_ptr.get() == component.get())
        {
            AttributeEditorMap::iterator attributeIter = attributeEditors_.begin();
            while(attributeIter != attributeEditors_.end())
            {
                IAttribute *attribute = comp_ptr->GetAttribute(attributeIter.value()->GetAttributeName());
                if(attribute)
                    attributeIter.value()->RemoveComponent(component);
                attributeIter++;
            }
            components_.erase(iter);
            break;
        }
        iter++;
    }
    UpdateGroupPropertyText();
}

void ECComponentEditor::UpdateUi()
{
    for(AttributeEditorMap::iterator iter = attributeEditors_.begin();
        iter != attributeEditors_.end();
        iter++)
    {
        iter.value()->UpdateEditorUI();
    }
}

void ECComponentEditor::OnEditorChanged(const QString &name)
{
    PROFILE(ECComponentEditor_OnEditorChanged);
    ECAttributeEditorBase *editor = qobject_cast<ECAttributeEditorBase*>(sender());
    if(!editor)
    {
        ECEditorModule::LogWarning("Fail to convert signal sender to ECAttributeEditorBase.");
        return;
    }
    groupProperty_->addSubProperty(editor->GetProperty());
}

QString ECComponentEditor::GetAttributeType(const QString &name) const
{
    AttributeEditorMap::const_iterator iter = attributeEditors_.find(name);
    if (iter != attributeEditors_.end())
        return (*iter)->GetAttributeType();
    return QString();
}
