// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_AvatarAppearance.h"
#include "Renderer.h"

namespace RexLogic
{
    bool AvatarAsset::IsLoaded() const
    {
        // If contains no resource id, then we assume it's a local asset which are considered "always loaded"
        if (resource_id_.empty())
            return true;
        if (resource_)
            return true;
        return false;
    }
    
    void AvatarAsset::ResourceLoaded(Foundation::ResourcePtr resource)
    {
        if (!resource)
            return;
        if ((resource->GetId() == resource_id_) && (resource->GetType() == resource_type_))
            resource_ = resource;
    }
    
    void AvatarAsset::RequestRendererResource(OgreRenderer::Renderer* renderer, Core::RequestTagVector& tags)
    {
        if ((!resource_id_.empty()) && (!resource_type_.empty()))
        {
            Core::request_tag_t tag = renderer->RequestResource(resource_id_, resource_type_);
            if (tag)
                tags.push_back(tag);
        }
    }
    
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
        ClearProperties();
        
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
    
    bool EC_AvatarAppearance::AreResourcesLoaded() const
    {
        if (!mesh_.IsLoaded())
            return false;
        if (!skeleton_.IsLoaded())
            return false;
        for (Core::uint i = 0; i < materials_.size(); ++i)
        {
            if (!materials_[i].asset_.IsLoaded())
                return false;
            for (Core::uint j = 0; j < materials_[i].textures_.size(); ++j)
            {
                if (!materials_[i].textures_[j].IsLoaded())
                    return false;
            }
        }
        for (Core::uint i = 0; i < attachments_.size(); ++i)
        {
            if (!attachments_[i].mesh_.IsLoaded())
                return false;
        }
        
        return true;
    }
    
    void EC_AvatarAppearance::ResourceLoaded(Foundation::ResourcePtr resource)
    {
        // Kind of lazy code, just check every avatar asset and see which one matches (if any)
        mesh_.ResourceLoaded(resource);
        skeleton_.ResourceLoaded(resource);
        
        for (Core::uint i = 0; i < materials_.size(); ++i)
        {
            materials_[i].asset_.ResourceLoaded(resource);
            
            for (Core::uint j = 0; j < materials_[i].textures_.size(); ++j)
            {
                materials_[i].textures_[j].ResourceLoaded(resource);
            }
        }
        for (Core::uint i = 0; i < attachments_.size(); ++i)
        {
            attachments_[i].mesh_.ResourceLoaded(resource);
        }
    }
    
    void EC_AvatarAppearance::RequestRendererResources(OgreRenderer::Renderer* renderer, Core::RequestTagVector& tags)
    {
        mesh_.RequestRendererResource(renderer, tags);
        skeleton_.RequestRendererResource(renderer, tags);
        
        for (Core::uint i = 0; i < materials_.size(); ++i)
        {
            materials_[i].asset_.RequestRendererResource(renderer, tags);
            
            for (Core::uint j = 0; j < materials_[i].textures_.size(); ++j)
            {
                materials_[i].textures_[j].RequestRendererResource(renderer, tags);
            }
        }
        for (Core::uint i = 0; i < attachments_.size(); ++i)
        {
            attachments_[i].mesh_.RequestRendererResource(renderer, tags);
        }
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
