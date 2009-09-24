// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_AvatarAppearance.h"
#include "RexLogicModule.h"

namespace RexLogic
{
    const std::string& AvatarAsset::GetLocalOrResourceName() const
    {
        if (resource_)
            return resource_->GetId();
        else
            return name_;
    }
    
    EC_AvatarAppearance::EC_AvatarAppearance(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
    }

    EC_AvatarAppearance::~EC_AvatarAppearance()
    {
    }
    
    void EC_AvatarAppearance::SetMesh(const AvatarAsset& mesh)
    {
        mesh_ = mesh;
    }
    
    void EC_AvatarAppearance::SetSkeleton(const AvatarAsset& skeleton)
    {
        skeleton_ = skeleton;
    }
    
    void EC_AvatarAppearance::SetMaterials(const AvatarMaterialVector& materials)
    {
        materials_ = materials;
    }
    
    void EC_AvatarAppearance::SetBoneModifiers(const BoneModifierSetVector& modifiers)
    {
        bone_modifiers_ = modifiers;
    }
    
    void EC_AvatarAppearance::SetMorphModifiers(const MorphModifierVector& modifiers)
    {
        morph_modifiers_ = modifiers;
    }
    
    void EC_AvatarAppearance::SetAnimations(const AnimationDefinitionMap& animations)
    {
        animations_ = animations;
    }
    
    void EC_AvatarAppearance::SetAttachments(const AvatarAttachmentVector& attachments)
    {
        attachments_ = attachments;
    }
    
    void EC_AvatarAppearance::SetTransform(const Transform& transform)
    {
        transform_ = transform;
    }
    
    void EC_AvatarAppearance::SetProperty(const std::string& name, const std::string& value)
    {
        properties_[name] = value;
    }
    
    void EC_AvatarAppearance::SetMorphModifierValue(const std::string& name, Core::Real value)
    {
        for (unsigned i = 0; i < morph_modifiers_.size(); ++i)
        {
            if (morph_modifiers_[i].name_ == name)
            {
                morph_modifiers_[i].value_ = value;
                break;
            }
        }
    }
    
    void EC_AvatarAppearance::SetBoneModifierSetValue(const std::string& name, Core::Real value)
    {
        for (unsigned i = 0; i < bone_modifiers_.size(); ++i)
        {
            if (bone_modifiers_[i].name_ == name)
            {
                bone_modifiers_[i].value_ = value;
                break;
            }
        }
    }
    
    void EC_AvatarAppearance::ClearProperties()
    {
        properties_.clear();
    }
    
    void EC_AvatarAppearance::Clear()
    {
        AvatarAsset blank_asset;
        Transform identity;
        
        mesh_ = blank_asset;
        skeleton_ = blank_asset;
        transform_ = identity;
        materials_.clear();
        animations_.clear();
        bone_modifiers_.clear();
        morph_modifiers_.clear();
        properties_.clear();
        asset_map_.clear();
    }
    
    const std::string& EC_AvatarAppearance::GetProperty(const std::string& name) const
    {
        static const std::string empty_property;
        
        AvatarPropertyMap::const_iterator i = properties_.find(name);
        if (i != properties_.end())
            return i->second;
        
        else return empty_property;
    }
    
    bool EC_AvatarAppearance::HasProperty(const std::string& name) const
    {
        if (properties_.find(name) != properties_.end())
            return true;
        else
            return false;
    }
    
    const AnimationDefinition& GetAnimationByName(const AnimationDefinitionMap& animations, const std::string& name)
    {
        static AnimationDefinition default_def;
        
        AnimationDefinitionMap::const_iterator def = animations.begin();
        while (def != animations.end())
        {
            if (def->second.animation_name_ == name)
                return def->second;
            ++def;
        }
        return default_def;
    }
}
