#include "StableHeaders.h"
#include "ComponentGroup.h"
#include "ECComponentEditor.h"
#include "Entity.h"
#include "EC_DynamicComponent.h"

#include <ComponentInterface.h>

#include <QtProperty>

namespace ECEditor
{
    ComponentGroup::ComponentGroup(Foundation::ComponentInterfacePtr component,
                                 ECComponentEditor *editor,
                                 QTreeWidgetItem *listItem,
                                 bool isDynamic):
        editor_(editor),
        browserListItem_(listItem),
        isDynamic_(isDynamic),
        name_(""),
        typeName_("")
    {
        assert(component.get());
        // We assume that first component has been already added to the component editor when it was created.
        if(component.get())
        {
            components_.push_back(Foundation::ComponentWeakPtr(component));
            name_ = component->Name();
            typeName_ = component->TypeName();
        }
    }

    ComponentGroup::~ComponentGroup()
    {
        SAFE_DELETE(editor_)
    }

    //! Checks if components type and name are same.
    bool ComponentGroup::IsSameComponent(const Foundation::ComponentInterface &component) const
    {
        if(!IsValid())
            return false;
        /*if(components_[0].expired())
            return false;
        // Make sure that attribute type and name is same in both components.
        Foundation::ComponentInterface *myComponent = components_[0].lock().get();*/
        if(component.TypeName() != typeName_ ||
           component.Name() != name_)
            return false;

        //Include extra check for dynamic component, so that we can be sure that their name, typename and attributes are exactly same.
        if(isDynamic_)
        {
            if(components_[0].expired())
                return false;
            const EC_DynamicComponent &thisComponent = dynamic_cast<const EC_DynamicComponent&>(*components_[0].lock().get());
            const EC_DynamicComponent &compareComponent = dynamic_cast<const EC_DynamicComponent&>(component);
            if(!(&compareComponent) || !(&thisComponent))
                return false;
            
            if(!thisComponent.ContainSameAttributes(compareComponent))
                return false;
        }
        return true;
    }

    bool ComponentGroup::ContainComponent(const Foundation::ComponentInterface *component) const
    {
        for(uint i = 0; i < components_.size(); i++)
        {
            if(components_[i].expired())
                continue;
            if(components_[i].lock().get() == component)
                return true;
        }
        return false;
    }

    bool ComponentGroup::ContainAttribute(const std::string &name) const
    {
        if(components_.empty())
            return false;

        for(uint i = 0; i < components_.size(); i++)
        {
            if(components_[i].expired())
                continue;
            if(components_[i].lock()->GetAttribute(name))
                return true;
        }
        return false;
    }

    bool ComponentGroup::AddComponent(Foundation::ComponentInterfacePtr comp)
    {
        //Check if the component have already added to component group.
        if(ContainComponent(comp.get())) 
            return false;
        components_.push_back(Foundation::ComponentWeakPtr(comp));
        editor_->AddNewComponent(comp, false);
        return true;
    }

    bool ComponentGroup::RemoveComponent(Foundation::ComponentInterface *comp)
    {
        if(!ContainComponent(comp))
            return false;

        std::vector<Foundation::ComponentWeakPtr>::iterator iter = components_.begin();
        for(; iter != components_.end(); iter++)
        {
            if(iter->expired())
                continue;
            if(iter->lock()->GetParentEntity()->GetId() == comp->GetParentEntity()->GetId())
            {
                editor_->RemoveComponent(comp);
                components_.erase(iter);
                if(!editor_->ComponentsCount())
                    SAFE_DELETE(editor_);
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

    /*bool ComponentGroup::operator< (const ComponentGroup &comp)
    {
        if(comp.typeName_ < typeName_)
            return true;
        if(comp.name_ < name_)
            return true;
        return false;
    }

    bool ComponentGroup::operator== (const ComponentGroup &comp)
    {
        if(comp.name_ == name_ && comp.typeName_ == typeName_)
            return true;
        return false;
    }*/
}
