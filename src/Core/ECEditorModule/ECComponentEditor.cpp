// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECComponentEditor.h"
#include "ECEditorModule.h"
#include "ECEditorWindow.h"

#include "IAttribute.h"
#include "ECAttributeEditor.h"
#include "IComponent.h"
#include "Transform.h"
#include "AssetReference.h"
#include "AttributeMetadata.h"
#include "LoggingFunctions.h"
#include "Profiler.h"
#include "SceneAPI.h"

#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtProperty>

#include <QSize>
#include <QPoint>

#include "MemoryLeakCheck.h"

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
    foreach(IAttribute *attr, component->Attributes())
    {
        if (!attr)
            continue;
        
        ECAttributeEditorBase *attributeEditor = ECComponentEditor::CreateAttributeEditor(propertyBrowser_, this, attr);
        if (!attributeEditor)
            continue;
            
        attributeEditors_[attr->Name()] = attributeEditor;
        groupProperty_->setToolTip("Component type is " + component->TypeName());
        groupProperty_->addSubProperty(attributeEditor->Property());
        connect(attributeEditor, SIGNAL(AttributeAboutToBeEdited(IAttribute*)), this, SIGNAL(AttributeAboutToBeEdited(IAttribute *)));
        connect(attributeEditor, SIGNAL(EditorChanged(const QString &)), this, SLOT(OnEditorChanged(const QString &)));
    }
}

void ECComponentEditor::UpdateGroupPropertyText()
{
    if(!groupProperty_ || !components_.size())
        return;

    QString groupPropertyName = IComponent::EnsureTypeNameWithoutPrefix(typeName_);

    if(!name_.isEmpty())
        groupPropertyName += ": " + name_;
    if(components_.size() > 1)
        groupPropertyName += QString(" (%1 components)").arg(components_.size());
    
    /// @todo This starts to get a little hard to read with all this data in the field.
    /// Icons or more columns would be nice here, but untill then this is what we get.
    int temporary = 0;
    int local = 0;
    ComponentSet::iterator iter = components_.begin();
    while(iter != components_.end())
    {
        ComponentPtr comp_ptr = (*iter).lock();
        if (comp_ptr.get())
        {
            if (comp_ptr->IsTemporary())
                temporary++;
            if (comp_ptr->IsLocal())
                local++;
        }
        ++iter;
    }
    if (temporary > 0 || local > 0)
    {
        groupPropertyName.append(" [");
        if (components_.size() == 1)
        {
            if (local > 0)
                groupPropertyName.append("Local");
            if (local > 0 && temporary > 0)
                groupPropertyName.append(" ");
            if (temporary > 0)
                groupPropertyName.append("Temporary");
        } 
        else if (components_.size() > 1)
        {
            if (local > 0 && local < components_.size())
                groupPropertyName.append(QString("%1/%2 is Local").arg(local).arg(components_.size()));
            else if (local > 0 && local == components_.size())
                groupPropertyName.append(QString("%1 Local").arg(local));
            if (local > 0 && temporary > 0)
                groupPropertyName.append(" ");
            if (temporary > 0 && temporary < components_.size())
                groupPropertyName.append(QString("%1/%2 is Temporary").arg(temporary).arg(components_.size()));
            else if (temporary > 0 && local == components_.size())
                groupPropertyName.append(QString("%1 Temporary").arg(temporary));
        }
        groupPropertyName.append("]");
    }

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
    /// Check that component type is same as editor's typename (We only want to add same type of components to editor).
    if (component->TypeName() != typeName_)
        return;

    components_.push_back(ComponentWeakPtr(component));
    /// insert new component to each attribute editor.
    AttributeEditorMap::iterator iter = attributeEditors_.begin();
    while(iter != attributeEditors_.end())
    {
        IAttribute *attribute = component->AttributeByName(iter.value()->AttributeName());
        if(attribute)
            iter.value()->AddComponent(component);
        iter++;
    }
    UpdateGroupPropertyText();

    // Update UI if any attributes metadata changes.
    connect(component.get(), SIGNAL(AttributeMetadataChanged(IAttribute*, const AttributeMetadata*)), this, SLOT(UpdateUi()), Qt::UniqueConnection);
}

void ECComponentEditor::RemoveComponent(ComponentPtr component)
{
    if(!component)
        return;

    if(component->TypeName() != typeName_)
        return;

    disconnect(component.get(), SIGNAL(AttributeMetadataChanged(IAttribute*, const AttributeMetadata*)), this, SLOT(UpdateUi()));

    ComponentSet::iterator iter = components_.begin();
    while(iter != components_.end())
    {
        ComponentPtr comp_ptr = (*iter).lock();
        if(comp_ptr.get() == component.get())
        {
            AttributeEditorMap::iterator attributeIter = attributeEditors_.begin();
            while(attributeIter != attributeEditors_.end())
            {
                IAttribute *attribute = comp_ptr->AttributeByName(attributeIter.value()->AttributeName());
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

void ECComponentEditor::RemoveAttribute(ComponentPtr comp, IAttribute *attr)
{
    if (!comp)
        return;
    if (comp->TypeName() != typeName_)
        return;

    ComponentSet::const_iterator iter = components_.begin();
    while(iter != components_.end())
    {
        ComponentPtr component = (*iter).lock();
        if (component == comp)
        {
            AttributeEditorMap::iterator attributeIter = attributeEditors_.begin();
            while(attributeIter != attributeEditors_.end())
            {
                IAttribute *attribute = component->AttributeByName(attributeIter.value()->AttributeName());
                if (attribute == attr)
                {
                    SAFE_DELETE(attributeIter.value())
                    attributeEditors_.erase(attributeIter);
                    return;
                }
                else
                    ++attributeIter;
            }
        }
        ++iter;
    }
}

void ECComponentEditor::UpdateUi()
{
    for(AttributeEditorMap::iterator iter = attributeEditors_.begin(); iter != attributeEditors_.end(); ++iter)
        iter.value()->UpdateEditorUI();
}

QString ECComponentEditor::GetAttributeType(const QString &name) const
{
    AttributeEditorMap::const_iterator iter = attributeEditors_.find(name);
    if (iter != attributeEditors_.end())
        return (*iter)->AttributeTypeName();
    return QString();
}

void ECComponentEditor::OnEditorChanged(const QString & /*name*/)
{
    PROFILE(ECComponentEditor_OnEditorChanged);
    ECAttributeEditorBase *editor = qobject_cast<ECAttributeEditorBase*>(sender());
    if(!editor)
    {
        LogWarning("Fail to convert signal sender to ECAttributeEditorBase.");
        return;
    }
    groupProperty_->addSubProperty(editor->Property());
}

ECAttributeEditorBase *ECComponentEditor::CreateAttributeEditor(
    QtAbstractPropertyBrowser *browser,
    ECComponentEditor *editor,
    IAttribute *attribute)
{
    switch(attribute->TypeId())
    {
    case cAttributeString: return new ECAttributeEditor<QString>(browser, attribute, editor);
    case cAttributeInt: return new ECAttributeEditor<int>(browser, attribute, editor);
    case cAttributeReal: return new ECAttributeEditor<float>(browser, attribute, editor);
    case cAttributeColor: return new ECAttributeEditor<Color>(browser, attribute, editor);
    case cAttributeFloat2: return new ECAttributeEditor<float2>(browser, attribute, editor);
    case cAttributeFloat3: return new ECAttributeEditor<float3>(browser, attribute, editor);
    case cAttributeFloat4: return new ECAttributeEditor<float4>(browser, attribute, editor);
    case cAttributeBool: return new ECAttributeEditor<bool>(browser, attribute, editor);
    case cAttributeUInt: return new ECAttributeEditor<uint>(browser, attribute, editor);
    case cAttributeQuat: return new ECAttributeEditor<Quat>(browser, attribute, editor);
    case cAttributeAssetReference: return  new AssetReferenceAttributeEditor(browser, attribute, editor); // Note: AssetReference uses own special case editor.
    case cAttributeAssetReferenceList: return new AssetReferenceListAttributeEditor(browser, attribute, editor); // Note: AssetReferenceList uses own special case editor.
    case cAttributeEntityReference: return new ECAttributeEditor<EntityReference>(browser, attribute, editor);
    case cAttributeQVariant: return new ECAttributeEditor<QVariant>(browser, attribute, editor);
    case cAttributeQVariantList: return new ECAttributeEditor<QVariantList>(browser, attribute, editor);
    case cAttributeTransform: return new ECAttributeEditor<Transform>(browser, attribute, editor);
    case cAttributeQPoint: return new ECAttributeEditor<QPoint>(browser, attribute, editor);
    default:
        LogWarning("ECComponentEditor::CreateAttributeEditor: Unknown attribute type '" + attribute->TypeName() + "'.");
        return 0;
    }
}
