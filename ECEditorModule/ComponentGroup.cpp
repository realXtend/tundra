// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ComponentGroup.h"
#include "ECComponentEditor.h"
#include "Entity.h"
#include "EC_DynamicComponent.h"
#include "IComponent.h"

#include <QtProperty>

ComponentGroup::ComponentGroup(ComponentPtr component,
                               ECComponentEditor *editor,
                               bool isDynamic):
    editor_(editor),
    isDynamic_(isDynamic)
{
    assert(component);
    // No point to add component to editor cause it's already added in ECBrowser's AddNewComponentToGroup mehtod.
    if(component)
    {
        components_.push_back(ComponentWeakPtr(component));
        name_ = component->Name();
        typeName_ = component->TypeName();
    }
}

ComponentGroup::~ComponentGroup()
{
    SAFE_DELETE(editor_)
}

//! Checks if components type and name are same.
//! @todo made some major changes to this mehtod ensure that everything is working right.
bool ComponentGroup::IsSameComponent(ComponentPtr component) const
{
    assert(component);
    if(!IsValid())
        return false;

    PROFILE(ComponentGroup_IsSameComponent);
    if(component->TypeName() != typeName_ ||
       component->Name() != name_)
        return false;

    // If component type is dynamic component we need to compere their attributes aswell. To ensure
    // that they are holding exactly the same attributes.
    if(isDynamic_)
    {
        EC_DynamicComponent *thisComponent = dynamic_cast<EC_DynamicComponent*>(components_[0].lock().get());
        EC_DynamicComponent *compareComponent = dynamic_cast<EC_DynamicComponent*>(component.get());
        if(!compareComponent || !thisComponent)
            return false;

        if(!thisComponent->ContainSameAttributes(*compareComponent))
            return false;
    }
    return true;
}

bool ComponentGroup::ContainsComponent(ComponentPtr component) const
{
    for(uint i = 0; i < components_.size(); i++)
    {
        if(components_[i].expired())
            continue;
        if(components_[i].lock().get() == component.get())
            return true;
    }
    return false;
}

bool ComponentGroup::ContainsAttribute(const QString &name) const
{
    if(components_.empty())
        return false;

    for(uint i = 0; i < components_.size(); i++)
    {
        ComponentPtr comp = components_[i].lock();
        if(comp && comp->GetAttribute(name))
            return true;
    }
    return false;
}

bool ComponentGroup::AddComponent(ComponentPtr comp)
{
    PROFILE(ComponentGroup_AddComponent);
    //Check if the component have already added to component group or it's name or type are different for the component group.
    if(ContainsComponent(comp) || comp->Name() != name_ || comp->TypeName() != typeName_) 
        return false;
    components_.push_back(ComponentWeakPtr(comp));
    editor_->AddNewComponent(comp);
    return true;
}

bool ComponentGroup::RemoveComponent(ComponentPtr comp)
{
    if (!ContainsComponent(comp))
        return false;

    std::vector<ComponentWeakPtr>::iterator iter = components_.begin();
    for(; iter != components_.end(); iter++)
    {
        ComponentPtr comp_ptr = iter->lock();
        if (comp.get() == comp_ptr.get())
        {
            editor_->RemoveComponent(comp);
            components_.erase(iter);
            return true;
        }
    }
    return false;
}

//! Check if spesific QtProperty is owned by this component.
bool ComponentGroup::HasRootProperty(QtProperty *property) const 
{ 
    return editor_->RootProperty() == property;
}