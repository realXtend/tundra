// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_EC_AvatarAppearance_h
#define incl_RexLogic_EC_AvatarAppearance_h

#include "ComponentInterface.h"
#include "ResourceInterface.h"
#include "RexTypes.h"
#include "RexUUID.h"

namespace RexLogic
{
   //! Avatar asset name map (key: human-readable name, value: resource id)
    typedef std::map<std::string, std::string> AvatarAssetMap;
    
    //! Defines an asset for an avatar
    class AvatarAsset
    {
    public:
        //! Asset (human-readable) name
        std::string name_;
        //! Asset resource id
        std::string resource_id_;
        //! Resource (once it has been loaded)
        Foundation::ResourcePtr resource_;
        
        const std::string& GetLocalOrResourceName() const;
    };
    
    typedef std::vector<AvatarAsset> AvatarAssetVector;
    
    //! Defines a material for an avatar, with n textures
    struct AvatarMaterial
    {
        //! Material script asset
        AvatarAsset asset_;
        //! Textures used by material
        /*! Note: a good asset reference system would take care of them automatically, but can't rely on that at least with the old avatarstorage.
            Also, we have the case of using a material, but with custom textures. In this case the material resource has to be cloned and modified.
         */
        AvatarAssetVector textures_;
    };
    
    //! Material vector. Materials are to be stored in submesh order (ie. index 0 is submesh 0)
    typedef std::vector<AvatarMaterial> AvatarMaterialVector;
    
    //! Defines a transform for an avatar, attachment or bone
    struct Transform
    {
        Core::Vector3df position_;
        Core::Quaternion orientation_;
        Core::Vector3df scale_;
        
        Transform() : position_(0.0f, 0.0f, 0.0f),
            scale_(1.0f, 1.0f, 1.0f)
        {
        }
    };
    
    //! Possible modes for a bone modification
    enum ModifierMode
    {
        //! Relative to the bone's initial transform
        Relative = 0,
        //! Absolute, overrides the bone's initial transform
        Absolute,
        //! Cumulative, adds to a previous modifier
        Cumulative
    };
    
    //! Defines a bone modifier
    struct BoneModifier
    {
        //! Name of bone in avatar skeleton
        std::string bone_name_;
        //! Start transform
        Transform start_;
        //! End transform
        Transform end_;
        //! Mode of applying position modification
        ModifierMode position_mode_;
        //! Mode of applying rotation modification
        ModifierMode orientation_mode_;

        BoneModifier() : position_mode_(Relative),
            orientation_mode_(Relative)
        {
        }
    };
    
    typedef std::vector<BoneModifier> BoneModifierVector;
    
    //! Defines a set of bone modifiers (also called a dynamic animation)
    struct BoneModifierSet
    {
        //! Identifying human-readable name, not mandatory and not used directly in code
        std::string name_;
        //! Individual bone modifiers
        BoneModifierVector modifiers_;
        //! Interpolation value between start (0.0) and end (1.0) transform of individual modifiers
        Core::Real value_;
        
        BoneModifierSet() : value_(0.0f)
        {
        }
    };
    
    typedef std::vector<BoneModifierSet> BoneModifierSetVector;
    
    //! Defines a morph modifier
    struct MorphModifier
    {
        //! Identifying human-readable name, not mandatory and not used directly in code
        std::string name_;
        //! Name of morph animation
        std::string morph_name_;
        //! Interpolation value between none (0.0) and full (1.0) influence
        Core::Real value_;
        
        MorphModifier() : value_(0.0f)
        {
        }
    };
    
    typedef std::vector<MorphModifier> MorphModifierVector;
    
    //! Defines an animation for an avatar
    struct AnimationDefinition
    {
        //! Most likely a UUID
        std::string id_;
        //! Identifying human-readable name, not mandatory and not used directly in code
        std::string name_;
        //! Actual animation name in the mesh/skeleton
        std::string animation_name_;
        //! Should play looped?
        bool looped_;
        //! Exclusive; override (stop) other animations
        bool exclusive_;
        //! Speed scaled with avatar movement speed?
        bool use_velocity_;
        //! Always restart animation when it starts playing?
        bool always_restart_;
        //! Blend-in period in seconds
        Core::Real fadein_;
        //! Blend-out period in seconds
        Core::Real fadeout_;
        //! Speed modification (1.0 original)
        Core::Real speedfactor_;
        //! Weight modification (1.0 full)
        Core::Real weightfactor_;
        
        AnimationDefinition() :
            looped_(true),
            exclusive_(false),
            use_velocity_(false),
            always_restart_(false),
            fadein_(0.0),
            fadeout_(0.0),
            speedfactor_(1.0),
            weightfactor_(1.0)
        {
        }
    };
    
    typedef std::map<RexTypes::RexUUID, AnimationDefinition> AnimationDefinitionMap;
    
    typedef std::map<std::string, std::string> AvatarPropertyMap;
    
    //! Defines an attachment for an avatar
    struct AvatarAttachment
    {
        //! Name of attachment
        std::string name_;
        //! Mesh 
        AvatarAsset mesh_;
        //! Whether skeleton should be linked (for animations)
        bool link_skeleton_;
        //! Materials used by the attachment mesh
        AvatarMaterialVector materials_;
        //! Transform 
        Transform transform_;
        //! Category of attachment
        std::string category_;
        //! Base bone of attachment. Empty if attached directly to avatar scene node
        std::string bone_name_;
        //! Polygons indices to hide from avatar when using this attachment
        std::vector<Core::uint> vertices_to_hide_;
    };
    
    typedef std::vector<AvatarAttachment> AvatarAttachmentVector;
    
    const AnimationDefinition& GetAnimationByName(const AnimationDefinitionMap& animations, const std::string& name);

    //! Entity component that stores an avatar's appearance parameters
    class EC_AvatarAppearance : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_AvatarAppearance);
    public:
        virtual ~EC_AvatarAppearance();
        
        void SetMesh(const AvatarAsset& mesh);
        void SetSkeleton(const AvatarAsset& skeleton);
        void SetMaterials(const AvatarMaterialVector& materials);
        void SetBoneModifiers(const BoneModifierSetVector& modifiers);
        void SetMorphModifiers(const MorphModifierVector& modifiers);
        void SetAnimations(const AnimationDefinitionMap& animations);
        void SetAttachments(const AvatarAttachmentVector& attachments);
        void SetTransform(const Transform& transform);
        void SetProperty(const std::string& name, const std::string& value);
        void SetMorphModifierValue(const std::string& name, Core::Real value);
        void SetBoneModifierSetValue(const std::string& name, Core::Real value);
        void ClearProperties();
        void Clear();
        
        const AvatarAsset& GetMesh() const { return mesh_; }
        const AvatarAsset& GetSkeleton() const { return skeleton_; }
        const AvatarMaterialVector& GetMaterials() const { return materials_; }
        const BoneModifierSetVector& GetBoneModifiers() const { return bone_modifiers_; }
        const MorphModifierVector& GetMorphModifiers() const { return morph_modifiers_; }
        const AnimationDefinitionMap& GetAnimations() const { return animations_; }
        const AvatarAttachmentVector& GetAttachments() const { return attachments_; }
        const Transform& GetTransform() const { return transform_; }
        bool HasProperty(const std::string& name) const;
        const std::string& GetProperty(const std::string& name) const;
        
        const AvatarAssetMap& GetAssetMap() const { return asset_map_; }
        void SetAssetMap(const AvatarAssetMap& map) { asset_map_ = map; }
        
    private:
        EC_AvatarAppearance(Foundation::ModuleInterface* module);
        
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
        Transform transform_;
        //! Bone modifiers
        BoneModifierSetVector bone_modifiers_;
        //! Morph modifiers
        MorphModifierVector morph_modifiers_;
        //! Avatar asset name map
        AvatarAssetMap asset_map_;
        //! Miscellaneous properties
        AvatarPropertyMap properties_;
    };
}

#endif
