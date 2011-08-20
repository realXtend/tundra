// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "EntityComponent/EC_AvatarAppearance.h"

// Deprecated. Functionality replaced by AvatarDescAsset.
/*

const std::string& AvatarAsset::GetLocalOrResourceName() const
{
    return name_;
}

EC_AvatarAppearance::EC_AvatarAppearance(IModule* module) :
    IComponent(module->GetFramework())
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

void EC_AvatarAppearance::SetTransform(const AvatarTransform& transform)
{
    transform_ = transform;
}

void EC_AvatarAppearance::SetProperty(const std::string& name, const std::string& value)
{
    properties_[name] = value;
}
    
void EC_AvatarAppearance::SetMasterModifierValue(const std::string& name, float value)
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

void EC_AvatarAppearance::SetModifierValue(const std::string& name, AppearanceModifier::ModifierType type, float value)
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
    AvatarTransform identity;
    
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

*/

