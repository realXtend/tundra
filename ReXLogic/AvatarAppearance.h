// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarAppearance_h
#define incl_RexLogic_AvatarAppearance_h

#include "EC_AvatarAppearance.h"

class QDomDocument;

namespace Ogre
{
    class Bone;
    class Entity;
    class Node;
    class Vector3;
    class Quaternion;
}

namespace RexLogic
{
    class RexLogicModule;
    
    //! Handles setting up and updating avatars' appearance. Used internally by RexLogicModule::Avatar.
    class AvatarAppearance
    {
    public:
        AvatarAppearance(RexLogicModule *rexlogicmodule);
        ~AvatarAppearance();
        
        //! Reads default appearance of avatar from file to xml document
        void ReadDefaultAppearance(const std::string& filename);
        
        //! Sets up an avatar entity's appearance. 
        /*! Since this involves deserializing the appearance description XML & creating the mesh entity,
            should only be called when the whole appearance changes. Calls also SetupDynamicAppearance().
         */
        void SetupAppearance(Scene::EntityPtr entity);
        
        //! Sets ups the dynamic part of an avatar's appearance. This includes morphs & bone modifiers.
        void SetupDynamicAppearance(Scene::EntityPtr entity);
        
        //! Adjusts (possibly dynamic) height offset of avatar
        void AdjustHeightOffset(Scene::EntityPtr entity);
        
    private:
        //! Sets up an avatar mesh
        void SetupMeshAndMaterials(Scene::EntityPtr entity);
        
        //! Sets up avatar morphs
        void SetupMorphs(Scene::EntityPtr entity);
        
        //! Sets up avatar bone modifiers
        void SetupBoneModifiers(Scene::EntityPtr entity);
        
        //! Sets up avatar attachments
        void SetupAttachments(Scene::EntityPtr entity);
        
        //! Resets mesh entity bones to initial transform from the mesh original skeleton
        void ResetBones(Scene::EntityPtr entity);
        
        //! Applies a bone modifier
        void ApplyBoneModifier(Scene::EntityPtr entity, const BoneModifier& modifier, Core::Real value);
        
        //! Hides vertices from an entity's mesh. Mesh should be cloned from the base mesh and this must not be called more than once for the entity.
        void HideVertices(Ogre::Entity*, std::set<Core::uint> vertices_to_hide);
        
        //! Gets a bone safely from the avatar skeleton
        /*! \return Pointer to bone, or 0 if does not exist
         */
        Ogre::Bone* GetAvatarBone(Scene::EntityPtr entity, const std::string& bone_name);
        
        //! Gets initial derived transform of a bone. This is something Ogre can't give us automatically
        void GetInitialDerivedBonePosition(Ogre::Node* bone, Ogre::Vector3& position);
        
        //! Default avatar appearance xml document
        boost::shared_ptr<QDomDocument> default_appearance_;
        
        RexLogicModule *rexlogicmodule_;
    };
}

#endif
