// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_AvatarAppearance.h"
#include "Renderer.h"
#include "OgreMaterialResource.h"
#include "OgreMeshResource.h"
#include "OgreImageTextureResource.h"
#include "OgreMaterialUtils.h"
#include "RexLogicModule.h"

namespace RexLogic
{
    void AvatarAsset::SetResource(Foundation::ResourcePtr resource, const AvatarAssetMap& asset_map)
    {
        if (!resource)
            return;
        AvatarAssetMap::const_iterator i = asset_map.find(name_);
        if (i == asset_map.end())
            return;
        if (resource->GetId() == i->second)
        {
            resource_id_ = resource->GetId();
            resource_ = resource;
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
    
    void EC_AvatarAppearance::SetResource(Foundation::ResourcePtr resource)
    {
        // Kind of lazy code, just check every avatar asset and see which one matches (if any)
        mesh_.SetResource(resource, asset_map_);
        skeleton_.SetResource(resource, asset_map_);
       
        for (Core::uint i = 0; i < materials_.size(); ++i)
        {
            materials_[i].asset_.SetResource(resource, asset_map_);
            
            for (Core::uint j = 0; j < materials_[i].textures_.size(); ++j)
            {
                materials_[i].textures_[j].SetResource(resource, asset_map_);
            }
        }
        for (Core::uint i = 0; i < attachments_.size(); ++i)
        {
            attachments_[i].mesh_.SetResource(resource, asset_map_);
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
    
    void EC_AvatarAppearance::FixupResources(OgreRenderer::Renderer* renderer)
    {
        // Fix avatar mesh materials
        for (Core::uint i = 0; i < materials_.size(); ++i)
            FixupMaterial(materials_[i], renderer);
        // Get & fix attachment mesh materials
        for (Core::uint i = 0; i < attachments_.size(); ++i)
        {
            if (attachments_[i].mesh_.resource_)
            {
                OgreRenderer::OgreMeshResource* mesh_res = dynamic_cast<OgreRenderer::OgreMeshResource*>(attachments_[i].mesh_.resource_.get());
                if (mesh_res)
                {
                    const Core::StringVector& attach_matnames = mesh_res->GetOriginalMaterialNames();
                    attachments_[i].materials_.clear();
                    AvatarMaterial attach_newmat;
                    for (Core::uint j = 0; j < attach_matnames.size(); ++j)
                    {
                        attach_newmat.asset_.name_ = attach_matnames[i];
                        FixupMaterial(attach_newmat, renderer);
                        attachments_[i].materials_.push_back(attach_newmat);
                    }
                }
            }
        }
    }
    
    void EC_AvatarAppearance::FixupMaterial(AvatarMaterial& mat, OgreRenderer::Renderer* renderer)
    {
        // First thing to do: append .material to name if it doesn't exist, because our asset map is based on full asset name
        std::string fixed_mat_name = mat.asset_.name_;
        if (fixed_mat_name.find(".material") == std::string::npos)
            fixed_mat_name.append(".material");
        
        // First find resource for the material itself
        if (!mat.asset_.resource_)
        {
            mat.asset_.resource_id_ = asset_map_[fixed_mat_name];
            if (!mat.asset_.resource_id_.empty())
                mat.asset_.resource_ = renderer->GetResource(mat.asset_.resource_id_, OgreRenderer::OgreMaterialResource::GetTypeStatic());
        }
        // If couldn't be found, abort
        if (!mat.asset_.resource_)
            return;
        
        OgreRenderer::OgreMaterialResource* mat_res = dynamic_cast<OgreRenderer::OgreMaterialResource*>(mat.asset_.resource_.get());
        if (!mat_res)
            return;
            
        const Core::StringVector& orig_textures = mat_res->GetOriginalTextureNames();
        if (mat.textures_.size() < orig_textures.size())
            mat.textures_.resize(orig_textures.size());
        for (Core::uint i = 0; i < mat.textures_.size(); ++i)
        {
            if (!mat.textures_[i].resource_)
            {
                // Fill in name if not specified
                if ((mat.textures_[i].name_.empty()) && (i < orig_textures.size()))
                    mat.textures_[i].name_ = orig_textures[i];
                mat.textures_[i].resource_id_ = asset_map_[mat.textures_[i].name_];
                
                if (!mat.textures_[i].resource_id_.empty())
                    mat.textures_[i].resource_ = renderer->GetResource(mat.textures_[i].resource_id_, OgreRenderer::OgreImageTextureResource::GetTypeStatic());
            }
            // If we found the texture, modify the material to use it. Note: if there are any resource clashes, should be trivial to clone the material here to allow unique overrides
            if (mat.textures_[i].resource_)
            {
                Ogre::MaterialPtr ogremat = mat_res->GetMaterial();
                OgreRenderer::SetTextureUnitOnMaterial(ogremat, mat.textures_[i].GetLocalOrResourceName(), i);
            }
        }
    }
}
