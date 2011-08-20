// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_EC_AvatarAppearance_h
#define incl_RexLogic_EC_AvatarAppearance_h

#include "IComponent.h"
#include "IAsset.h"
#include "RexTypes.h"
#include "RexUUID.h"
#include "AvatarModuleApi.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "Quaternion.h"

//! Avatar asset name map (key: human-readable name, value: resource id)
typedef std::map<std::string, std::string> AvatarAssetMap;

// Deprecated. Replaced with Tundra Asset API.
//! Defines an asset for an avatar
/*
class AV_MODULE_API AvatarAsset
{
public:
    //! Asset (human-readable) name
    std::string name_;
    //! Asset resource id
    std::string resource_id_;
    
    const std::string& GetLocalOrResourceName() const;
};

typedef std::vector<AvatarAsset> AvatarAssetVector;
*/

// Deprecated. Materials are always used. For texture overrides, a new material needs to be generated.
/*
//! Defines a material for an avatar, with n textures
struct AV_MODULE_API AvatarMaterial
{
    //! Material script asset
    AvatarAsset asset_;
    //! Textures used by material
    AvatarAssetVector textures_;
};

//! Material vector. Materials are to be stored in submesh order (ie. index 0 is submesh 0)
typedef std::vector<AvatarMaterial> AvatarMaterialVector;
*/

// Deprecated. Functionality replaced by AvatarDescAsset.
/*
//! Entity component that stores an avatar's appearance parameters
class AV_MODULE_API EC_AvatarAppearance : public IComponent
{
    Q_OBJECT
        
    DECLARE_EC(EC_AvatarAppearance);
public:
    virtual ~EC_AvatarAppearance();
    
    void SetMesh(const AvatarAsset& mesh);
    void SetSkeleton(const AvatarAsset& skeleton);
    void SetMaterials(const AvatarMaterialVector& materials);
    void SetBoneModifiers(const BoneModifierSetVector& modifiers);
    void SetMorphModifiers(const MorphModifierVector& modifiers);
    void SetMasterModifiers(const MasterModifierVector& modifiers);
    void SetAnimations(const AnimationDefinitionMap& animations);
    void SetAttachments(const AvatarAttachmentVector& attachments);
    void SetTransform(const AvatarTransform& transform);
    void SetProperty(const std::string& name, const std::string& value);
    void SetMasterModifierValue(const std::string& name, float value);
    void SetModifierValue(const std::string& name, AppearanceModifier::ModifierType, float value);
    void ClearProperties();
    void Clear();
    
    const AvatarAsset& GetMesh() const { return mesh_; }
    const AvatarAsset& GetSkeleton() const { return skeleton_; }
    const AvatarMaterialVector& GetMaterials() const { return materials_; }
    const BoneModifierSetVector& GetBoneModifiers() const { return bone_modifiers_; }
    const MorphModifierVector& GetMorphModifiers() const { return morph_modifiers_; }
    const MasterModifierVector& GetMasterModifiers() const { return master_modifiers_; }
    const AnimationDefinitionMap& GetAnimations() const { return animations_; }
    const AvatarAttachmentVector& GetAttachments() const { return attachments_; }
    const AvatarTransform& GetTransform() const { return transform_; }
    bool HasProperty(const std::string& name) const;
    const std::string& GetProperty(const std::string& name) const;
    const AvatarPropertyMap& GetProperties() const { return properties_; }
    
    const AvatarAssetMap& GetAssetMap() const { return asset_map_; }
    void SetAssetMap(const AvatarAssetMap& map) { asset_map_ = map; }
    
    //! Recalculate effect of master modifiers
    void CalculateMasterModifiers();
    
private:
    EC_AvatarAppearance(IModule* module);
    AppearanceModifier* FindModifier(const std::string& name, AppearanceModifier::ModifierType type);
    
    //! Avatar mesh
    AvatarAsset mesh_;
    //! Avatar skeleton
    AvatarAsset skeleton_;
    //! Avatar materials
    AvatarMaterialVector materials_;
    //! Animation defines
    AnimationDefinitionMap animations_;
    //! Attachments
    AvatarAttachmentVector attachments_;
    //! Transform
    AvatarTransform transform_;
    //! Bone modifiers
    BoneModifierSetVector bone_modifiers_;
    //! Morph modifiers
    MorphModifierVector morph_modifiers_;
    //! Master modifiers
    MasterModifierVector master_modifiers_; 
    //! Avatar asset name map
    AvatarAssetMap asset_map_;
    //! Miscellaneous properties
    AvatarPropertyMap properties_;
};
*/

#endif
