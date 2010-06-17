// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EntityComponent/EC_AvatarAppearance.h"
#include "RexLogicModule.h"

namespace RexLogic
{
    const std::string& AvatarAsset::GetLocalOrResourceName() const
    {
        if (resource_.lock().get())
            return resource_.lock()->GetId();
        else
            return name_;
    }
    
    EC_AvatarAppearance::EC_AvatarAppearance(Foundation::ModuleInterface* module)
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

    void EC_AvatarAppearance::SetMasterModifiers(const MasterModifierVector& modifiers)
    {
        master_modifiers_ = modifiers;
        
        // Sort the mappings to ascending master position for correct interpolation
        for (uint i = 0; i < master_modifiers_.size(); ++i)
        {
            for (uint j = 0; j < master_modifiers_[i].modifiers_.size(); ++j)
            {
                std::sort(master_modifiers_[i].modifiers_[j].mapping_.begin(), master_modifiers_[i].modifiers_[j].mapping_.end());
            }
        }   
             
        CalculateMasterModifiers();
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
        
    void EC_AvatarAppearance::SetMasterModifierValue(const std::string& name, Real value)
    {
        if (value < 0.0) value = 0.0;
        if (value > 1.0) value = 1.0;
        
        for (uint i = 0; i < master_modifiers_.size(); ++i)
        {
            if (master_modifiers_[i].name_ == name)
            {
                master_modifiers_[i].value_ = value;
                for (uint j = 0; j < master_modifiers_[i].modifiers_.size(); ++j)
                {
                    AppearanceModifier* mod = FindModifier(master_modifiers_[i].modifiers_[j].name_, master_modifiers_[i].modifiers_[j].type_);                
                    if (mod)
                        mod->manual_ = false;
                }
                CalculateMasterModifiers();
                return;
            }
        }                 
    }
    
    void EC_AvatarAppearance::SetModifierValue(const std::string& name, AppearanceModifier::ModifierType type, Real value)
    {
        if (value < 0.0) value = 0.0;
        if (value > 1.0) value = 1.0;
        
        AppearanceModifier* mod = FindModifier(name, type);
        if (mod)
        {
            mod->value_ = value;
            mod->manual_ = true;
        }
    }
    
    void EC_AvatarAppearance::CalculateMasterModifiers()
    {
        for (uint i = 0; i < morph_modifiers_.size(); ++i)
            morph_modifiers_[i].ResetAccumulation();

        for (uint i = 0; i < bone_modifiers_.size(); ++i)
            bone_modifiers_[i].ResetAccumulation();

        for (uint i = 0; i < master_modifiers_.size(); ++i)
        {
            for (uint j = 0; j < master_modifiers_[i].modifiers_.size(); ++j)
            {
                AppearanceModifier* mod = FindModifier(master_modifiers_[i].modifiers_[j].name_, master_modifiers_[i].modifiers_[j].type_);
                if (mod)
                {
                    float slave_value = master_modifiers_[i].modifiers_[j].GetMappedValue(master_modifiers_[i].value_);
                    
                    mod->AccumulateValue(slave_value, master_modifiers_[i].modifiers_[j].mode_ == SlaveModifier::Average);
                }
            }
        }          
    }    
    
    AppearanceModifier* EC_AvatarAppearance::FindModifier(const std::string& name, AppearanceModifier::ModifierType type)
    {
        for (uint i = 0; i < morph_modifiers_.size(); ++i)
        {
            if ((morph_modifiers_[i].name_ == name) && (morph_modifiers_[i].type_ == type))
                return &morph_modifiers_[i];
        }
        for (uint i = 0; i < bone_modifiers_.size(); ++i)
        {
            if ((bone_modifiers_[i].name_ == name) && (bone_modifiers_[i].type_ == type))
                return &bone_modifiers_[i];
        }
        return 0;
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
        master_modifiers_.clear();
        attachments_.clear();
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
    
    void AppearanceModifier::ResetAccumulation()
    {
        sum_ = 0.0f;
        samples_ = 0;
    }
    
    void AppearanceModifier::AccumulateValue(Real value, bool use_average)
    {
        sum_ += value;
        samples_++;
 
        if (manual_)
            return;
                    
        if (!use_average)
        {
            value_ = sum_;
            if (value_ < 0.0f) value_ = 0.0f;
            if (value_ > 1.0f) value_ = 1.0f;
        }
        else
        {
            value_ = sum_ / samples_;
            if (value_ < 0.0f) value_ = 0.0f;
            if (value_ > 1.0f) value_ = 1.0f;
        }            
    }
                
    Real SlaveModifier::GetMappedValue(Real master_value)
    {
        // If no positions to interpolate, map master slider directly to modifier pos
        if (mapping_.size() < 2)
        {
            return master_value;
        }

        // Find out the minimum/maximum range of supported master positions
        float min_value = 1.0f;
        float max_value = 0.0f;
        uint i;
        
        for (i = 0; i < mapping_.size(); ++i)
        {
            if (mapping_[i].master_ < min_value)
                min_value = mapping_[i].master_;
            if (mapping_[i].master_ > max_value)
                max_value = mapping_[i].master_;
        }

        // Now cap the master position according to what is supported
        if (master_value < min_value)   
            master_value = min_value;
        if (master_value > max_value)
            master_value = max_value;
            
        // Find beginning pos. of interpolation
        for (i = mapping_.size()-1; i >= 0; --i)
        {
            if (mapping_[i].master_ <= master_value)
                break;
        }

        // If at the endpoint, simply return the value at end
        if (i == mapping_.size()-1)
        {
            return mapping_[i].slave_;
        }

        float delta = mapping_[i+1].slave_ - mapping_[i].slave_;
        float master_delta = mapping_[i+1].master_ - mapping_[i].master_;
        float weight = 0.0f;
        if (master_delta > 0.0f)
        {
            weight = (master_value - mapping_[i].master_) / master_delta;
        }

        return mapping_[i].slave_ + weight * delta;
    }
}
